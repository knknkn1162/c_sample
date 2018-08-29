#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "msg.h"


int main(int argc, char *argv[]) {
  struct sockaddr_un svaddr, claddr;
  size_t msgLen;
  ssize_t num;
  char resp[RESP_MSG_SIZE];
  int sfd;

  sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  memset(&claddr, 0, sizeof(struct sockaddr_un));
  claddr.sun_family = AF_UNIX;
  snprintf(claddr.sun_path, sizeof(claddr.sun_path), CLIENT_SOCK_TEMPLATE, (long)getpid());


  // passive response
  if(bind(sfd, (struct sockaddr*)&claddr, sizeof(struct sockaddr_un)) == -1) {
    perror("bind");
    exit(1);
  }

  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SERVER_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

  if(sendto(sfd, argv[1], strlen(argv[1]), 0, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) != strlen(argv[1])) {
    perror("sendto");
    exit(1);
  }

  num = recvfrom(sfd, resp, RESP_MSG_SIZE, 0, NULL, NULL);
  if(num == -1) {
    perror("recvfrom");
    exit(1);
  }

  if(write(STDOUT_FILENO, resp, num) != num) {
    perror("write");
    exit(1);
  }
  write(STDOUT_FILENO, "\n", 1);

  remove(claddr.sun_path);

  return 0;
}
