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
  int sfd;
  ssize_t num;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  char port[10];
  char buf[RESP_MSG_SIZE];

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;

  snprintf(port, 10, "%d", PORT_NUM);
  if(getaddrinfo("localhost", port, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  for(rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(sfd == -1) {
      perror("socket");
      exit(1);
    }

    printf("rp: ai_family: %d, socktype: %d, protocol: %d\n", rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    if(connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }
  }
  freeaddrinfo(result);
  
  if(rp == NULL) {
    perror("could not connect");
    exit(1);
  }


  if(write(sfd, argv[1], strlen(argv[1])) != strlen(argv[1])) {
    perror("write");
    exit(1);
  }
  printf("[client] send message\n");

  num = read(sfd, buf, RESP_MSG_SIZE);
  if(num == -1) {
    perror("read");
    exit(1);
  }
  

  if(write(STDOUT_FILENO, buf, num) != num) {
    perror("write");
    exit(1);
  }
  write(STDOUT_FILENO, "\n", 1);

  return 0;
}
