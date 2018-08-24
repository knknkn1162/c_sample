#include "msg.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int serverFd, clientFd, dummyFd;
  struct sigaction sa;

  char clientFifo[CLIENT_FIFO_NAME_LEN];

  umask(0);
  if(mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
    perror("mkfifo");
    exit(1);
  }
  serverFd = open(SERVER_FIFO, O_RDONLY);
  if(serverFd == -1) {
    perror("open");
  }
  // open an extra write descriptor, so that we never see EOF
  dummyFd = open(SERVER_FIFO, O_WRONLY);
  if(dummyFd == -1) {
    perror("open");
    exit(1);
  }


  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SIG_IGN;
  // If the server attempts to write to a client FIFO that doesn't have a reader, then rather than being sent a SIGPIPE signal, it receives an EPIPE error from the write system call.
  if(sigaction(SIGPIPE, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    struct request req;
    struct response resp;
    // never see EOF because of the dummyFd.
    if(read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
      fprintf(stderr, "discarding\n");
      continue;
    }

    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)req.clientId);
    clientFd = open(clientFifo, O_WRONLY);

    if(clientFd == -1) {
      perror("open");
      exit(1);
    }


    resp.mtype = RESP_DATA;
    strncpy(resp.message, req.pathName, strlen(req.pathName));

    if(write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response)) {
      fprintf(stderr, "server write\n");
    }

    if(close(clientFd) == -1) {
      perror("close");
    }
  }
}
