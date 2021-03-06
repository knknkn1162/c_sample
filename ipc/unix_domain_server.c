#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "msg.h"


int main(int argc, char *argv[]) {
  int sfd;
  struct sockaddr_un addr;
  char buf[REQ_MSG_SIZE];

  sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(sfd == -1) {
    perror("socket");
    exit(1);
  }

  // remove sock_file if existed
  // Note that after the server terminates, the socket pathname continues to exist. This is why the server uses remove() to remove any existing instance of the socket pathname before calling bind().
  if(remove(SERVER_SOCK_PATH) == -1 && errno != ENOENT) {
    perror("remove");
    exit(1);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SERVER_SOCK_PATH, sizeof(addr.sun_path) - 1);

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

    // receive request
    while((numRead = read(cfd, buf, RESP_MSG_SIZE)) > 0) {
      // send response
      if(write(cfd, buf, numRead) != numRead) {
        perror("write");
        exit(1);
      }
    }

    if(close(cfd) == -1) {
      perror("close");
      exit(1);
    }
  }

  remove(addr.sun_path);
  return 0;
}
