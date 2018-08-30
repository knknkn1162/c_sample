#define _DEFAULT_SOURCE
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h> // struct addrinfo
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "msg.h"
#include <string.h>


int main(int argc, char *argv[]) {
  struct sockaddr_in svaddr, claddr;
  int sfd;
  ssize_t num;
  char buf[REQ_MSG_SIZE];
  char claddrStr[INET_ADDRSTRLEN];
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  char port[10];


  // call getaddrinfo
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  snprintf(port, 10, "%d", PORT_NUM);
  if(getaddrinfo(NULL, port, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  for(rp = result; rp != NULL; rp = rp->ai_next) {
    int optval = 1;
    printf("rp: ai_family: %d, socktype: %d, protocol: %d\n", rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(sfd == -1) {
      continue;
    }
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if(bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
  }
  freeaddrinfo(result);
  // int socket(int domain, int type, int protocol);
  /* sfd = socket(AF_INET, SOCK_DGRAM, 0); */
  /* if(sfd == -1) { */
    /* perror("socket"); */
    /* exit(1); */
  /* } */
  /* memset(&svaddr, 0, sizeof(struct sockaddr_in)); */
  /* svaddr.sin_family = AF_INET; */
  /* svaddr.sin_port = htons(PORT_NUM); */
  /* svaddr.sin_addr.s_addr = htonl(INADDR_ANY); */

  /* // passive */
  /* if(bind(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_in)) == -1) { */
    /* perror("bind"); */
    /* exit(1); */
  /* } */

  while(1) {
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    socklen_t len = sizeof(struct sockaddr_in);
    printf("[server] wait request\n");
    num = recvfrom(sfd, buf, REQ_MSG_SIZE, 0, (struct sockaddr*)&claddr, &len);
    if(num == -1) {
      perror("recvfrom");
      exit(1);
    }

    // for display
    // int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);
    if(getnameinfo((struct sockaddr*)&claddr, len, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
      printf("Connection from %s %s\n", host, service);
    } else {
      printf("conneciton Unknown\n");
    }

    // const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
    if(inet_ntop(AF_INET, &claddr.sin_addr, claddrStr, INET_ADDRSTRLEN) == NULL) {
      printf("Couldn't convert \n");
      exit(1);
    }
    printf("[server]claddrStr: %s, port: %u\n", claddrStr, ntohs(claddr.sin_port));

    if(sendto(sfd, buf, num, 0, (struct sockaddr*)&claddr, len) != num) {
      perror("sendto");
      exit(1);
    }
  }

  return 0;
}

