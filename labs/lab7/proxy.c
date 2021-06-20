#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000 // 1MB
#define MAX_OBJECT_SIZE 102400 // 100kB
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
void make_request_message(char *header, char *host, char *path, rio_t *rp);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

// functions for cache
int find_cache(char *uri);
int choose_victim(void);
void update_cache_priority(int index);
void update_cache_content(char *uri, char *buf);

typedef struct {
  int priority;
  char cache_content[MAX_OBJECT_SIZE];
  char cache_url[MAXLINE];
} cache_node;

cache_node cache[10];

int main(int argc, char **argv) {
  int listenfd;
  pthread_t tid;
  struct sockaddr_storage clientaddr;
  socklen_t clientlen = sizeof(clientaddr);

  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  Signal(SIGPIPE, SIG_IGN);

  for (int i = 0; i < 10; i++) {
    cache[i].priority = i;
    *cache[i].cache_content = '\0';
    *cache[i].cache_url = '\0';
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    int *connfd = (int *)malloc(sizeof(int));
    *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Pthread_create(&tid, NULL, thread_per_connection, (void *)connfd);
  }
  return 0;
}

void *thread_per_connection(void *data) {
  int connfd = *(int *)data;
  Pthread_detach(pthread_self());
  free(data);

  handle_client(connfd);

  Close(connfd);
  return NULL;
}

void handle_client(int clientfd) {
  rio_t client_rio, server_rio;
  char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char request_to_server[MAXLINE];
  char buf[MAXLINE];

  int server_fd;

  char host[MAXLINE], path[MAXLINE];
  int port;
  char strport[MAXLINE];

  char cache_buf[MAX_OBJECT_SIZE];
  int buf_size = 0;
  int cached;
  size_t n;

  // read client request
  Rio_readinitb(&client_rio, clientfd);
  Rio_readlineb(&client_rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);

  // cWe only deal with GET
  if (strcasecmp(method, "GET")) {
    clienterror(clientfd, method, "501", "Not Implemented",
                "Proxy does not implement this method");
    return;
  }

  cached = find_cache(uri);
  if (cached != -1) {
    Rio_writen(clientfd, cache[cached].cache_content,
               strlen(cache[cached].cache_content));
    update_cache_priority(cached);
    return;
  }

  parse_uri(uri, host, path, &port);
  make_request_message(request_to_server, host, path, &client_rio);

  sprintf(strport, "%d", port);
  server_fd = Open_clientfd(host, strport);

  Rio_readinitb(&server_rio, server_fd);

  Rio_writen(server_fd, request_to_server, strlen(request_to_server));
  while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
    buf_size += n;
    if (buf_size < MAX_OBJECT_SIZE)
      strcat(cache_buf, buf);
    Rio_writen(clientfd, buf, n);
  }

  Close(server_fd);
  update_cache_content(uri, cache_buf);
}

void make_request_message(char *header, char *host, char *path, rio_t *rp) {
  char buf[MAXLINE];

  char request_header[MAXLINE]; // request
  char host_hdr[MAXLINE];

  // request line
  sprintf(request_header, "GET %s HTTP/1.0\r\n", path);

  // request headers
  while (Rio_readlineb(rp, buf, MAXLINE) > 0) {
    if (strcmp(buf, "\r\n") == 0) // EOF
      break;

    if (strncasecmp(buf, "Host", 4) == 0)
      strcpy(host_hdr, buf);
  }

  sprintf(header, "%s%s%s%s%s%s\r\n", request_header, host_hdr, user_agent_hdr,
          connection_hdr, proxy_connection_hdr, user_agent_hdr);

  return;
}

// parse uri and receive port, path, host, ...
void parse_uri(char *uri, char *host, char *path, int *port) {
  char uri_backup[MAXLINE];
  strcpy(uri_backup, uri);
  //  For example: http://www.google.com:80/index.html

  // prologue
  char *pos = strstr(uri, "//");
  if (pos == NULL)
    pos = uri;
  else
    pos = pos + 2;

  // port
  char *port_pos = strstr(pos, ":");
  char *path_pos = strstr(pos, "/");
  if (port_pos == NULL) {
    *port = DEFAULT_PORT;
  } else {
    *port_pos = '\0';
    sscanf(port_pos + 1, "%d", port);
  }

  *path_pos = '\0';
  strcpy(host, pos);
  *path_pos = '/';
  strcpy(path, path_pos);

  strcpy(uri, uri_backup);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg) {
  char buf[MAXLINE];
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));
}

int find_cache(char *uri) {
  for (int i = 0; i < 10; i++) {
    if (strcmp(uri, cache[i].cache_url) == 0)
      return i;
  }

  return -1;
}

int choose_victim(void) {
  for (int idx = 0; idx < 10; idx++) {
    if ((strlen(cache[idx].cache_content) == 0) || (cache[idx].priority == 9))
      return idx;
  }
  return -1;
}

void update_cache_priority(int index) {
  int original_priority = cache[index].priority;

  for (int i = 0; i < 10; i++) {
    if (cache[i].priority < original_priority)
      cache[i].priority++;
  }

  cache[index].priority = 0;
}

void update_cache_content(char *uri, char *buf) {
  int i = choose_victim();

  strcpy(cache[i].cache_url, uri);
  strcpy(cache[i].cache_content, buf);

  update_cache_priority(i);
}