#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"

int main(int argc, char *argv[]) {
  struct sigaction sa;
  struct sockaddr_storage claddr;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, cfd;
  char port[10];
  char buf[REQ_MSG_SIZE];
  if(argc > 1) {
    fprintf(stderr, "usage error\n");
    exit(1);
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if(sigaction(SIGPIPE, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  snprintf(port, 10, "%d", PORT_NUM);

  if(getaddrinfo(NULL, port, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  for(rp = result; rp != NULL; rp = rp->ai_next) {
    int optval;
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(sfd == -1) {
      continue;
    }

    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
      perror("setsockopt");
      exit(1);
    }

    if(bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      // success and break;
      break;
    }

    close(sfd);
  }
  freeaddrinfo(result);

  if(rp == NULL) {
    perror("socket");
    exit(1);
  }

  if(listen(sfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  while(1) {
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    int num;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(sfd, (struct sockaddr*)&claddr, &addrlen);
    if(cfd == -1) {
      perror("accept");
      continue;
    }

    if(getnameinfo((struct sockaddr*)&claddr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
      printf("connection from %s, %s\n", host, service);
    } else {
      printf("connection unknown");
    }

    while((num = read(cfd, buf, REQ_MSG_SIZE)) > 0) {
      // send response
      if(write(cfd, buf, num) != num) {
        perror("write");
        exit(1);
      }
    }

    if(close(cfd) == -1) {
      perror("close");
      exit(1);
    }
  }
}
