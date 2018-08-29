#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define SOCK_PATH "/tmp/mysock"
#define BUF_SIZE 100

#define BACKLOG 5

int main(int argc, char *argv[]) {
  int sfd;
  struct sockaddr_un addr;
  char buf[BUF_SIZE];

  sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  // remove sock_file if existed
  // Note that after the server terminates, the socket pathname continues to exist. This is why the server uses remove() to remove any existing instance of the socket pathname before calling bind().
  if(remove(SOCK_PATH) == -1 && errno != ENOENT) {
    perror("remove");
    exit(1);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

  if(bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
    perror("bind");
  }

  if(listen(sfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  while(1) {
    int numRead;
    // create a new socket and it is this new socket that is connected to the peer socket that performed the connect().
    // accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int cfd = accept(sfd, NULL, NULL);
    if(cfd == -1) {
      perror("accept");
      exit(1);
    }

    while((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
      if(numRead == -1) {
        perror("read");
        exit(1);
      }
      if(write(STDOUT_FILENO, buf, numRead) != numRead) {
        perror("write");
        exit(1);
      }
      write(STDOUT_FILENO, "\n", 1);
    }

    if(close(cfd) == -1) {
      perror("close");
      exit(1);
    }
  }
}
