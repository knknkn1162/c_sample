#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 100
#define SERVER_SOCK_PATH "/tmp/mysock"
#define CLIENT_SOCK_TEMPLATE "/tmp/client.%ld"
#define BACKLOG 5

int main(int argc, char *argv[]) {
  struct sockaddr_un svaddr;
  int sfd;
  int num;
  char buf[BUF_SIZE];

  if(argc < 2) {
    fprintf(stderr, "usage error\n");
    exit(1);
  }

  sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SERVER_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

  // active request
  // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
  if(connect(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) == -1) {
    perror("connect");
    exit(1);
  }

  if(write(sfd, argv[1], strlen(argv[1])) != strlen(argv[1])) {
    perror("write");
    exit(1);
  }

  // response
  if((num = read(sfd, buf, BUF_SIZE)) == -1) {
    perror("read");
  }

  // echo msg to client
  if(write(STDOUT_FILENO, buf, num) == -1) {
    perror("write");
  }
  write(STDOUT_FILENO, "\n", 1);
  
  return 0;
}
