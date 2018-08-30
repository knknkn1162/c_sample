#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"
#include <netdb.h> // struct addrinfo


int main(int argc, char *argv[]) {
  int sfd;
  ssize_t num;
  char resp[RESP_MSG_SIZE];
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  char port[10];


  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_NUMERICSERV;

  snprintf(port, 10, "%d", PORT_NUM);
  if(getaddrinfo("localhost", port, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  for(rp = result; rp != NULL; rp = rp->ai_next) {
    printf("rp: ai_family: %d, socktype: %d, protocol: %d\n", rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(sfd == -1) {
      continue;
    }
    break;
  }
  freeaddrinfo(result);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  /* sfd = socket(AF_INET, SOCK_DGRAM, 0); */
  /* if(sfd == -1) { */
    /* perror("socket"); */
    /* exit(1); */
  /* } */

  /* memset(&svaddr, 0, sizeof(struct sockaddr_in)); */
  /* svaddr.sin_family = AF_INET; */
  /* svaddr.sin_port = htons(PORT_NUM); */
  /* if(inet_pton(AF_INET, "127.0.0.1", &svaddr.sin_addr) <= 0) { */
    /* perror("inet_pton"); */
    /* exit(1); */
  /* } */

  // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
  if(sendto(sfd, argv[1], strlen(argv[1]), 0, rp->ai_addr, sizeof(struct sockaddr_in)) != strlen(argv[1])) {
    perror("[client] sendto");
    exit(1);
  }

  num = recvfrom(sfd, resp, RESP_MSG_SIZE, 0, NULL, NULL);
  if(num == -1) {
    perror("recvfrom");
    exit(1);
  }

  write(STDOUT_FILENO, "[client] ", 9);
  if(write(STDOUT_FILENO, resp, num) != num) {
    perror("write");
    exit(1);
  }
  write(STDOUT_FILENO, "\n", 1);

  return 0;
}

