#include <stdio.h>
#include "csapp.h"

void doit(int fd);
int parse_uri(char *uri, char *hostname, char *path, char *port);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  rio_writen(fd, buf, strlen(buf));
  rio_writen(fd, body, strlen(body));
}

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
int parse_uri(char *uri, char *hostname, char *path, char *port)
{
  // sscanf(uri, "http://%[^/]/%s", hostname, path);

  if (strncasecmp(uri, "http://", 7) == 0)
  {
    char *hostname_pointer = uri + 7;
    char *port_pointer = strchr(hostname_pointer, ':');
    char *path_pointer = strchr(hostname_pointer, '/');

    if (port_pointer == NULL)
    {
      strcpy(port, "80");
    }
    else
    {
      strncpy(port,
              port_pointer + 1,
              (strlen(port_pointer + 1) - strlen(path_pointer)));
    }
    strcpy(hostname, hostname_pointer);
    strcpy(path, path_pointer);

    return 1;
  }
  else
  {
    return -1;
  }
}

/*
 * justdoit - handle one HTTP request/response transaction
 */
void justdoit(int fd)
{
  int is_valid_uri;
  char buf[MAXLINE];
  char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char hostname[MAXLINE], path[MAXLINE];
  rio_t rio;

  rio_readinitb(&rio, fd);

  if (!rio_readlineb(&rio, buf, MAXLINE))
  {
    return;
  }

  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if (strcasecmp(method, "GET"))
  {
    clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
    return;
  }

  // is_valid_uri = parse_uri(uri, hostname, path);

  // if (!is_valid_uri)
  // {
  //   clienterror(fd, path, "403", "Forbidden", "Wrong path");
  //   return;
  // }

  // sprintf(buf, "%s %s %s", method, path, "HTTP/1.0");

  // rio_writen(, buf, strlen(buf));
}

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = accept(listenfd, (SA *)&clientaddr, &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    justdoit(connfd); // line:netp:tiny:doit
    Close(connfd);    // line:netp:tiny:close
  }
}
