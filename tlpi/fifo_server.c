#include <signal.h>
#include "fifo_seqnum.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(void) {
  int serverFd, dummyFd, clientFd;
  char clientFifo[CLIENT_FIFO_NAME_LEN];
  struct request req;
  struct response resp;
  int seqNum = 0;

  umask(0);
  if(mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
    printf("mkfifo %s\n", SERVER_FIFO);
    exit(1);
  }

  serverFd = open(SERVER_FIFO, O_RDONLY);
  if(serverFd == -1) {
    printf("open %s\n", SERVER_FIFO);
    exit(1);
  }

  dummyFd = open(SERVER_FIFO, O_WRONLY);
  if(dummyFd == -1) {
    perror("open SERVER_INFO");
    exit(1);
  }

  while(1) {
    if(read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
      fprintf(stderr, "err reading reqest\n");
      continue;
    }

    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)req.pid);
    clientFd = open(clientFifo, O_WRONLY);

    resp.seqNum = seqNum;
    if(write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response)) {
      fprintf(stderr, "error writing to FIFO\n");
    }
    if(close(clientFd) == -1) {
      perror("close");
      exit(1);
    }

    seqNum += req.seqLen;
  }

}
