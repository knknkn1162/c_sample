#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"


int main(int argc, char *argv[]) {
  int sfd;
  socklen_t len;
  char buf[REQ_MSG_SIZE];
  struct sockaddr_un svaddr, claddr;

  sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  if(remove(SERVER_SOCK_PATH) == -1 && errno != ENOENT) {
    perror("remove");
    exit(1);
  }

  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SERVER_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

  // passive request
  if(bind(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) == -1) {
    perror("bind");
    exit(1);
  }

  while(1) {
    int num;
    len = sizeof(struct sockaddr_un);
    // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    // flag: socket specific I/O feature
    num = recvfrom(sfd, buf, REQ_MSG_SIZE, 0, (struct sockaddr*)&claddr, &len);
    if(num == -1) {
      perror("recvfrom");
      exit(1);
    }

    printf("[server] received %ld bytes from %s\n", (long)num, claddr.sun_path);
    if(sendto(sfd, buf, num, 0, (struct sockaddr*)&claddr, len) != num) {
      perror("sendto");
      exit(1);
    }
  }

  return 0;
}
