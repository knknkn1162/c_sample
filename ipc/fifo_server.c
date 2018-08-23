#include "msg.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  int serverFd, dummyFd, clientFd;

  char clientFifo[CLIENT_FIFO_NAME_LEN];

  umask(0);
  if(mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
    perror("mkfifo");
    exit(1);
  }

  struct request req;
  struct response resp;

  while(1) {
    if(read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
      fprintf(stderr, "read");
      continue;
    }
  }
}
