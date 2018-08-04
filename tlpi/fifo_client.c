#include "fifo_seqnum.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

static char clientFifo[CLIENT_FIFO_NAME_LEN];

static void removeFifo(void) {
  unlink(clientFifo);
}

int main(int argc) {
  int serverFd, clientFd;
  struct request req;
  struct response resp;

  umask(0);
  snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)getpid());
  if(mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
    perror("mkfifo");
  }

  atexit(removeFifo);

  req.pid = getpid();
  req.seqLen = 3;

  serverFd = open(SERVER_FIFO, O_WRONLY);
  if(serverFd == -1) {
    perror("serverFd");
    exit(1);
  }

  if(write(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
    perror("cant write to server");
    exit(1);
  }

  clientFd = open(clientFifo, O_RDONLY);
  read(clientFd, &resp, sizeof(struct response));

  printf("%d\n", resp.seqNum);
  exit(EXIT_SUCCESS);

}
