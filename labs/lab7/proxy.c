#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// Default port is 80, but it should manage other ports as well if port is
// included in uri
#define DEFAULT_PORT 80

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close";

// functions for proxy
void *thread_per_connection(void *data);
void handle_client(int connfd);
void parse_uri(char *uri, char *host, char *path, int *port);
void http_header_constructor(char *header, char *host, char *path, int port,
                             rio_t *rp);
int connect_server(char *host, int port);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

// functions for cache
void cache_init();
int find_cache(char *uri);
int cache_to_remove();
void cache_priority_update(int index);
void cache_uri(char *uri, char *buf);
void reader_P(int i);
void reader_V(int i);
void write_P(int i);
void write_V(int i);

typedef struct {
  char method[MAXLINE];
  char uri[MAXLINE];
  char version[MAXLINE];
} request;

typedef struct {
  // basic element
  char cache_object[MAX_OBJECT_SIZE];
  char cache_url[MAXLINE];

  // for caching
  int priority;
  // to check conviniently
  int is_empty;

  // sem for cache
  sem_t mutex;

  // reader writer sem
  int read_cnt;
  sem_t read_cnt_mutex;
  int write_cnt;
  sem_t write_cnt_mutex;
  sem_t queue;

} cache_node;

typedef struct {
  cache_node cache_arr[10]; // Why 10? MAX_CACHE_SIZE / MAX_OBJECT_SIZE ~= 10
  int num_caches;           // for convinience
} cache_list;

// Cache as Global Variable
cache_list cache;

int main(int argc, char **argv) {


    // init element required for connections
    long int listenfd, connfd;
    socklen_t clientlen;
    pthread_t tid;
    struct sockaddr_storage clientaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
  }

  // init cache
  cache_init();

  // ignore signal or it would terminate (instruction in handout)
  Signal(SIGPIPE, SIG_IGN);

  // listen to port
  listenfd = Open_listenfd(argv[1]);

  // keep listening
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    // concurrency appears in here
    Pthread_create(&tid, NULL, thread_per_connection, (void *)connfd);
  }
  return 0;
}

// this thread will be activated for every connection
void *thread_per_connection(void *data) {
    long int connfd = *(long int *)data;
    Pthread_detach(pthread_self());

    handle_client(connfd);

    Close(connfd);
    return NULL;
}

// handle client: core function
void handle_client(int clientfd) {
    request client_request;
    rio_t client_rio, server_rio;

    char buf[MAXLINE];

    char host[MAXLINE];
    char path[MAXLINE];
    int port;

    // read client request
    Rio_readinitb(&client_rio, clientfd);
    Rio_readlineb(&client_rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", client_request.method, client_request.uri,
           client_request.version);

    // save original uri
    char uri_original[100];
    strcpy(uri_original, client_request.uri);

    // cWe only deal with GET
    if (strcasecmp(client_request.method, "GET"))
    {
        clienterror(clientfd, client_request.method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
  }

  // cache: don't need to access server
  int is_cached = find_cache(uri_original);
  if (is_cached != -1) {
    // found
    reader_P(is_cached);
    // don't know the reason but this part cannot be disentangled
    Rio_writen(clientfd, cache.cache_arr[is_cached].cache_object,
               strlen(cache.cache_arr[is_cached].cache_object));
    reader_V(is_cached);
    // update cache priority
    cache_priority_update(is_cached);
    return;
  }


  // parse uri
  parse_uri(client_request.uri, host, path, &port);

  // server sent query
  char http_header_sent_to_server[MAXLINE];

  // query that would be sent to end server
  http_header_constructor(http_header_sent_to_server, host, path, port,
                          &client_rio);

  // connect to server
  int server_fd = connect_server(host, port);
  // connection fail
  if (server_fd < 0)
    return;


  Rio_readinitb(&server_rio, server_fd);

  // write to server
  Rio_writen(server_fd, http_header_sent_to_server,
             strlen(http_header_sent_to_server));

  // receive from server and update cache
  char cache_buf[MAX_OBJECT_SIZE];
  int buf_size = 0;
  size_t n;
  while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
    buf_size += n;
    if (buf_size < MAX_OBJECT_SIZE) {
      strcat(cache_buf, buf);
    }

    Rio_writen(clientfd, buf, n);
  }

  // close connection to server
  Close(server_fd);

  // store into cache
  cache_uri(uri_original, cache_buf);
}

void http_header_constructor(char *header, char *host, char *path, int port,
                             rio_t *rp) {
  char buf[MAXLINE];

  char request_header[MAXLINE]; // request
  char host_header[MAXLINE];
  char additional_header[MAXLINE];

  // request line
  sprintf(request_header, "GET %s HTTP/1.0\r\n", path);
  // keep receive request line
  while (Rio_readlineb(rp, buf, MAXLINE) > 0) {
    if (strcmp(buf, "\r\n") == 0) // EOF
      break;

    // Host
    if (strncasecmp(buf, "Host", 4) == 0) {
      strcpy(host_header, buf);
      continue; // read next input
    }

    // additional request
    if ((strncasecmp(buf, "Connection", 10) != 0) &&
        (strncasecmp(buf, "Proxy-Connection", 16) != 0) &&
        (strncasecmp(buf, "User-Agent", 10) != 0)) {
      strcat(additional_header, buf);
    }
  }

  // no host?
  if (strlen(host_header) == 0)
    sprintf(host_header, "Host: %s\r\n", host);

  // concat all result into header pointer
  sprintf(header, "%s%s%s%s%s%s%s%s", request_header, host_header,
          user_agent_hdr, connection_hdr, proxy_connection_hdr, user_agent_hdr,
          additional_header, "\r\n");

  return;
}

// connect to server
int connect_server(char *host, int port) {
  char str[20];
  sprintf(str, "%d", port);
  return Open_clientfd(host, str);
}

// parse uri and receive port, path, host, ...
void parse_uri(char *uri, char *host, char *path, int *port) {
  // header part
  char *pos = strstr(uri, "//");

  if (pos == NULL)
    pos = uri;
  else
    pos = pos + 2;

  // port part
  char *pos2 = strstr(pos, ":");

  // No port -> use default port 80
  if (pos2 == NULL) {
    // port
    *port = DEFAULT_PORT;
    // path part
    pos2 = strstr(pos, "/");

    if (pos2 == NULL)
      sscanf(pos, "%s", host);

    else {
      *pos2 = '\0';
      sscanf(pos, "%s", host);
      *pos2 = '/';
      sscanf(pos2, "%s", path);
    }
  }

  // exists port in uri
  else {
    *pos2 = '\0';
    // read host
    sscanf(pos, "%s", host);
    // read port and path
    sscanf(pos2 + 1, "%d%s", port, path);
  }

  return;
}

void cache_init() {

  // empty at first
  cache.num_caches = 0;

  // init all arr
  for (int i = 0; i < 10; i++) {
    // all empty
    cache.cache_arr[i].is_empty = 1;
    // no priority initially
    cache.cache_arr[i].priority = 0;

    // set mutex
    Sem_init(&cache.cache_arr[i].mutex, 0, 1);
    Sem_init(&cache.cache_arr[i].read_cnt_mutex, 0, 1);
    // set cnt
    cache.cache_arr[i].read_cnt = 0;
    cache.cache_arr[i].write_cnt = 0;
    Sem_init(&cache.cache_arr[i].write_cnt_mutex, 0, 1);
    Sem_init(&cache.cache_arr[i].queue, 0, 1);
  }
}

void reader_P(int i) {
  // lock
  P(&cache.cache_arr[i].queue);
  P(&cache.cache_arr[i].read_cnt_mutex);

  // increase cnt
  cache.cache_arr[i].read_cnt++;

  // if first -> lock cache
  if (cache.cache_arr[i].read_cnt == 1) {
    P(&cache.cache_arr[i].mutex);
  }

  // unlock next reader
  V(&cache.cache_arr[i].read_cnt_mutex);
  V(&cache.cache_arr[i].queue);
}

void reader_V(int i) {
  // lock
  P(&cache.cache_arr[i].read_cnt_mutex);

  // reduce cnt
  cache.cache_arr[i].read_cnt--;

  // no more reader -> unlock cache
  if (cache.cache_arr[i].read_cnt == 0) {
    V(&cache.cache_arr[i].mutex);
  }
  V(&cache.cache_arr[i].read_cnt_mutex);
}

void write_P(int i) {
  P(&cache.cache_arr[i].write_cnt_mutex);
  cache.cache_arr[i].write_cnt++;
  if (cache.cache_arr[i].write_cnt == 1) {
    P(&cache.cache_arr[i].queue);
  }
  V(&cache.cache_arr[i].write_cnt_mutex);
  P(&cache.cache_arr[i].mutex);
}

void write_V(int i) {
  V(&cache.cache_arr[i].mutex);
  P(&cache.cache_arr[i].write_cnt_mutex);
  cache.cache_arr[i].write_cnt--;
  if (cache.cache_arr[i].write_cnt == 0) {
    V(&cache.cache_arr[i].queue);
  }
  V(&cache.cache_arr[i].write_cnt_mutex);
}

// check cache for uri
int find_cache(char *uri) {
  int i;
  for (i = 0; i < 10; i++) {
    reader_P(i);
    // in cache?
    // not empty and uri equal
    if ((cache.cache_arr[i].is_empty == 0) &&
        (strcmp(uri, cache.cache_arr[i].cache_url) == 0)) {
      break;
    }
    reader_V(i);
  }

  // not found
  if (i >= 10) {
    return -1;
  }
  // found -> return index
  return i;
}

// find least priority cache
int cache_to_remove() {
  int min = MAXLINE;
  int min_index = 0;
  // loop cache
  for (int i = 0; i < 10; i++) {
    reader_P(i);
    // empty cache node -> use this one
    if (cache.cache_arr[i].is_empty == 1) {
      min_index = i;
      reader_V(i);
      break;
    }
    // o.w. -> find least priority
    if (cache.cache_arr[i].priority < min) { /*if not empty choose the min LRU*/
      min_index = i;
      min = cache.cache_arr[i].priority;
      reader_P(i);
      continue;
    }
    reader_V(i);
  }

  return min_index;
}

// newly updated -> priority change
void cache_priority_update(int index) {

  write_P(index);

  // largest priority
  // this one has highest priority
  // don't know reason but initial setting of priority effects the performance.
  cache.cache_arr[index].priority = MAX_OBJECT_SIZE;

  write_V(index);

  for (int i = 0; i < 10; i++) {
    if (i != index) {
      write_P(i);
      // decrease priority of the other
      if (cache.cache_arr[i].is_empty == 0) {
        cache.cache_arr[i].priority--;
      }
      write_V(i);
    }
  }
}

// cache the uri into cache
void cache_uri(char *uri, char *buf) {

  // eliminate one to reset one
  int i = cache_to_remove();

  // will write -> lock write
  write_P(i);

  // overwrite i'th cache
  strcpy(cache.cache_arr[i].cache_object, buf);
  strcpy(cache.cache_arr[i].cache_url, uri);

  // now this one is full
  cache.cache_arr[i].is_empty = 0;

  write_V(i);

  // most recently updated
  cache_priority_update(i);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg) {
  char buf[MAXLINE];

  /* Print the HTTP response headers */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));
}