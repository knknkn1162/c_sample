#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>


static char clientFifo[CLIENT_FIFO_NAME_LEN];

static void removeFifo(void) {
  unlink(clientFifo);
}


int main(int argc, char *argv[]) {
  int serverFd, clientFd;
  struct request req;
  struct response resp;

  if(argc < 1) {
    perror("fifo_client");
    exit(1);
  }

  umask(0);
  snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)getpid());
  if(mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
    perror("mkfifo");
    exit(1);
  }

  if(atexit(removeFifo) != 0) {
    perror("atexit");
  }

  req.clientId = getpid();
  strncpy(req.pathName, argv[1], strlen(argv[1]));

  serverFd = open(SERVER_FIFO, O_WRONLY);
  if(serverFd == -1) {
    perror("open");
  }

  // send message
  if(write(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
    perror("write");
    exit(1);
  }

  clientFd = open(clientFifo, O_RDONLY);
  if(clientFd == -1) {
    perror("open");
  }

  if(read(clientFd, &resp, sizeof(struct response)) != sizeof(struct response)) {
    perror("read");
    exit(1);
  }

  switch(resp.mtype) {
    case RESP_DATA:
      printf("[child(%d)]> %s\n", getpid(), resp.message);
      break;
    case RESP_END:
      printf("[child(%d)]> end\n", getpid());
      break;
    case RESP_FAILURE:
      fprintf(stderr, "ERROR\n");
      break;
  }

  return 0;

}
