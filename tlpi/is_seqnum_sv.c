#define _DEFAULT_SOURCE
#include <netdb.h>
#include "is_seqnum.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BACKLOG 50
#define ADDRSTRLEN NI_MAXHOST + NI_MAXSERV + 10

int createAddrInfo(struct addrinfo* info);

int main(int argc, char *argv[]) {
  uint32_t seqNum;
  char reqLenStr[INT_LEN];
  char seqNumStr[INT_LEN];
  struct sockaddr_storage claddr;
  int lfd, cfd, optval, reqLen;
  socklen_t addrlen;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  char addrStr[ADDRSTRLEN];
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
  struct sigaction sa;

  seqNum = 0;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGPIPE, &sa, 0);

  createAddrInfo(&hints);
  // int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
  if(getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  optval = 1;
  for(rp = result; rp != NULL; rp = rp->ai_next) {

    struct sockaddr_in *addr = (struct sockaddr_in*)rp->ai_addr;
    // int socket(int domain, int type, int protocol);
    lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(lfd == -1) {
      continue;
    }

    // int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    if(setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if(bind(lfd, (struct sockaddr*)addr, rp->ai_addrlen) == 0) {
      char str[INET_ADDRSTRLEN];
      // const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
      inet_ntop(AF_INET, &addr->sin_addr, str, INET_ADDRSTRLEN);
      printf("end bind %s:%u\n", str, addr->sin_port);
      break;
    }

    close(lfd);
  }
  if(rp == NULL) {
    perror("could not bind socket");
  }
  freeaddrinfo(result);

  if(listen(lfd, BACKLOG) == -1) {
    perror("listen");
  }
  printf("listen..\n");

  while(1) {
    addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(lfd, (struct sockaddr*)&claddr, &addrlen);
    if(cfd == -1) {
      perror("cfd accept");
      continue;
    }
    printf("accept\n");

    if(getnameinfo((struct sockaddr*)&claddr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
      snprintf(addrStr, ADDRSTRLEN, "(%s, %s)", host, service);
    } else {
      snprintf(addrStr, ADDRSTRLEN, "(?UNKNOWN)");
    }

    printf("Connection from %s\n", addrStr);

    //         numRead = read(fd, &ch, 1);
    if(readLine(cfd, reqLenStr, INT_LEN) <= 0) {
      close(cfd);
      continue;
    }

    reqLen = atoi(reqLenStr);
    if(reqLen <= 0) {
      close(cfd);
      continue;
    }

    snprintf(seqNumStr, INT_LEN, "%d\n", seqNum);
    if(write(cfd, &seqNumStr, strlen(seqNumStr)) != strlen(seqNumStr)) {
      fprintf(stderr, "err on write");
    }

    seqNum += reqLen;
    if(close(cfd) == -1) {
      perror("close");
    }
    printf("end connection %d\n", seqNum);
  }
}

int createAddrInfo(struct addrinfo* hints) {
  memset(hints, 0, sizeof(struct addrinfo));
  hints->ai_canonname = NULL;
  hints->ai_addr = NULL;
  hints->ai_next = NULL;
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_family = AF_UNSPEC;
  hints->ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  return 0;
}
