#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"


int main(int argc, char *argv[]) {
  struct sockaddr_in svaddr;
  int sfd;
  size_t msgLen;
  ssize_t num;
  char resp[RESP_MSG_SIZE];

  sfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  memset(&svaddr, 0, sizeof(struct sockaddr_in));
  svaddr.sin_family = AF_INET;
  svaddr.sin_port = htons(PORT_NUM);
  if(inet_pton(AF_INET, "127.0.0.1", &svaddr.sin_addr) <= 0) {
    perror("inet_pton");
    exit(1);
  }

  if(sendto(sfd, argv[1], strlen(argv[1]), 0, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_in)) != strlen(argv[1])) {
    perror("sendto");
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

