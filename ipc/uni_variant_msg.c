#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "msg.h"

#define BUF_MSG_SIZE 256
// length, msg, pid
#define BUF_SIZE (BUF_MSG_SIZE + sizeof(long) + sizeof(size_t))

int main(int argc, char *argv[]) {

  int pfd[2]; // for client -> pipe -> server
  pid_t pid;

  if(argc < 2) {
    fprintf(stderr, "Usage error: ./a.out [pathName]\n");
    exit(1);
  }

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  // file descriptors are duplicated on each processes.
  if((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    size_t len;
    long pid;
    char* iter;
    char req[BUF_SIZE];
    // child
    if(close(pfd[0]) == -1) {
      perror("[child] close request read fd");
    }

    len = strlen(argv[1]) + sizeof(size_t) + sizeof(long);
    pid = (long)getpid();
    iter = req;
    memcpy(iter, &len, sizeof(size_t));
    iter += sizeof(size_t);
    memcpy(iter, &pid, sizeof(long));
    iter += sizeof(long);
    memcpy(iter, argv[1], strlen(argv[1]));
    //printf("req: %s\n", &req[sizeof(size_t) + sizeof(long)]);

    if(write(pfd[1], req, len) != len) {
      perror("[child] write");
      exit(1);
    }

    if(close(pfd[1]) == -1) {
      perror("[child] close request write");
    }

    printf("[child] exit\n");
    _exit(EXIT_SUCCESS);
  } else {
    //parent
    char req[BUF_SIZE];
    char *msg;

    if(close(pfd[1]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    while(1) {
      char* iter;
      long mtype;
      size_t len, msgLen;
      size_t numRead;
      if((numRead = read(pfd[0], &req, sizeof(req))) == -1) {
        perror("read");
        exit(1);
      } else if (numRead == 0) {
        printf("[parent] EOF\n");
        break;
      } else {
        iter = req;
        memcpy(&len, iter, sizeof(size_t));
        iter += sizeof(size_t);
        msgLen = len - sizeof(size_t) - sizeof(long);
        memcpy(&mtype, iter, sizeof(long));
        iter += sizeof(long);
        msg = malloc(msgLen+1);
        memcpy(msg, iter, msgLen);
        msg[msgLen] = '\n';
        // echo
        if(write(STDOUT_FILENO, msg, msgLen+1) != msgLen+1) {
          perror("[parent] write");
          exit(1);
        }
        free(msg);
      }
    }

    // end message
    if(close(pfd[0]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    wait(NULL);
    printf("[parent] exit\n");
    exit(EXIT_SUCCESS);
  }
}
