#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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

  // int socket(int domain, int type, int protocol);
  sfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  memset(&svaddr, 0, sizeof(struct sockaddr_in));
  svaddr.sin_family = AF_INET;
  svaddr.sin_port = htons(PORT_NUM);
  svaddr.sin_addr.s_addr = INADDR_ANY;

  if(bind(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_in)) == -1) {
    perror("bind");
    exit(1);
  }

  while(1) {
    socklen_t len = sizeof(struct sockaddr_in);
    num = recvfrom(sfd, buf, REQ_MSG_SIZE, 0, (struct sockaddr*)&claddr, &len);
    if(num == -1) {
      perror("recvfrom");
      exit(1);
    }

    // const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
    if(inet_ntop(AF_INET, &claddr.sin_addr, claddrStr, INET_ADDRSTRLEN) == NULL) {
      printf("Couldn't convert \n");
      exit(1);
    }
    printf("claddrStr: %s, port: %u", claddrStr, ntohs(claddr.sin_port));

    if(sendto(sfd, buf, num, 0, (struct sockaddr*)&claddr, len) != num) {
      perror("sendto");
      exit(1);
    }
  }

  return 0;
}

