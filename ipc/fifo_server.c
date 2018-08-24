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

  // exit the server cloned process without zombie
  sa.sa_flags = SA_NOCLDWAIT;
  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    struct request req;
    struct response resp;
    pid_t pid;
    int numRead;
    int ifd;
    char buf[RESP_MSG_SIZE];
    // never see EOF because of the dummyFd.
    if(read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
      fprintf(stderr, "discarding\n");
      continue;
    }

    if((pid = fork()) == -1) {
      perror("fork");
      exit(1);
    } else if (pid > 0) {
      printf("[server] delegate the cloned process\n");
      continue;
    }
    memset(&resp, 0, sizeof(struct response));
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)req.clientId);
    clientFd = open(clientFifo, O_WRONLY);

    if(clientFd == -1) {
      perror("open");
      exit(1);
    }

    printf("[client] receive reuqest by %ld\n", req.clientId);
    ifd = open(req.pathName, O_RDONLY);
    if(ifd == -1) {
      int savedErrno;
      resp.mtype = RESP_FAILURE;
      strncpy(resp.message, strerror(errno), strlen(strerror(errno)) + 1);
      savedErrno = errno;
      if(write(clientFd, &resp, sizeof(resp)) != sizeof(resp)) {
          perror("write");
          break;
      }
      errno = savedErrno;
      exit(EXIT_FAILURE);
    }

    // send response
    printf("[client] send response\n");
    while(1) {
      if((numRead = read(ifd, &buf, RESP_MSG_SIZE)) == -1) {
        perror("[client] read");
        continue;
      } else if(numRead == 0) {
        // end msg
        printf("[client] EOF\n");
        resp.mtype = RESP_END;
        write(clientFd, &resp, sizeof(resp));
        break;
      }
      resp.mtype = RESP_DATA;
      strncpy(resp.message, buf, numRead);
      if(write(clientFd, &resp, RESP_SIZE) != RESP_SIZE) {
        perror("write");
        break;
      }
    }

    if(close(clientFd) == -1) {
      perror("close");
    }
    // exit cloned process
    _exit(EXIT_SUCCESS);
  }

  return 0;
}
