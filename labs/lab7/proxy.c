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
void make_request_message(char *header, char *host, char *path, rio_t *rp);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) {

  // init element required for connections
  int listenfd;
  pthread_t tid;
  struct sockaddr_storage clientaddr;
  socklen_t clientlen = sizeof(clientaddr);

  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  Signal(SIGPIPE, SIG_IGN);

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
  char method[MAXLINE];
  char uri[MAXLINE];
  char version[MAXLINE];
  int server_fd;

  char buf[MAXLINE];

  char host[MAXLINE];
  char path[MAXLINE];
  int port;
  char strport[MAXLINE];

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

  // parse uri
  parse_uri(uri, host, path, &port);

  // server sent query
  char request_to_server[MAXLINE];

  // query that would be sent to end server
  make_request_message(request_to_server, host, path, &client_rio);

  sprintf(strport, "%d", port);
  server_fd = Open_clientfd(host, strport);

  Rio_readinitb(&server_rio, server_fd);

  // write to server
  Rio_writen(server_fd, request_to_server, strlen(request_to_server));

  // receive from server
  size_t n;
  while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0)
    Rio_writen(clientfd, buf, n);

  // close connection to server
  Close(server_fd);
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

  return;
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg) {
  char buf[MAXLINE];
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));
}