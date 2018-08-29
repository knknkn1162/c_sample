#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 100
#define SOCK_PATH "/tmp/mysock"

int main(int argc, char *argv[]) {
  struct sockaddr_un svaddr;
  int sfd;

  if(argc < 2) {
    fprintf(stderr, "usage error\n");
    exit(1);
  }

  printf("PID: %d\n", getpid());
  sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path) - 1);

  // connect the active socket(sockfd) to the listening socket(server)
  if(connect(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) == -1) {
    perror("connect");
    exit(1);
  }

  if(write(sfd, argv[1], strlen(argv[1])) != strlen(argv[1])) {
    perror("write");
    exit(1);
  }

  return 0;
}
