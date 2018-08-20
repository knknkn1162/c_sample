#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "msg.h"

#define BUF_SIZE 256

int main(int argc, char *argv[]) {

  int pfd[2]; // for client -> pipe -> server
  int rpfd[2];
  int numRead;
  pid_t pid;

  if(argc < 2) {
    fprintf(stderr, "Usage error: ./a.out [pathName]\n");
    exit(1);
  }

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  if(pipe(rpfd) == -1) {
    perror("pipe");
    exit(1);
  }

  // file descriptors are duplicated on each processes.
  if((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    struct request req;
    // child
    if(close(pfd[0]) == -1) {
      perror("[child] close request read fd");
    }

    if(close(rpfd[1]) == -1) {
      perror("[child] close response write fd");
    }

    // pack request message into `struct request`.
    memset(&req, 0, sizeof(req));
    req.clientId = getpid();
    // char *strncpy(char *s1, const char *s2, size_t n);
    strncpy(req.pathName, argv[1], strlen(argv[1]));
    // ssize_t write(int fd, const void *buf, size_t count); 
    if(write(pfd[1], &req, sizeof(req)) != sizeof(req)) {
      perror("[child] write");
      exit(1);
    }

    if(close(pfd[1]) == -1) {
      perror("[child] close request write");
    }

    // response
    while(1) {
      struct response res;
      if((numRead = read(rpfd[0], &res, sizeof(res))) < 0) {
        perror("read");
        exit(1);
      } else if (numRead == 0) {
        printf("[child] EOF\n");
        break;
      } else {
        printf("[child] read response msg: %p\n", res.data);
        printf("%c %c %c\n", res.data[0], res.data[1], res.data[2]);
        if(write(STDOUT_FILENO, res.data, res.len) != res.len) {
            perror("[child] write");
            exit(1);
        }
      }
    }

    if(close(rpfd[0]) == -1) {
      perror("close");
      exit(1);
    }

    printf("[child] exit\n");
    _exit(EXIT_SUCCESS);
  } else {
    //parent
    int fd;
    char buf[BUF_SIZE];
    struct request req;

    printf("[parent] PID=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
    if(close(pfd[1]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    if(close(rpfd[0]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    while(1) {
      struct response res;
      if((numRead = read(pfd[0], &req, sizeof(req))) == -1) {
        perror("read");
      } else if (numRead == 0) {
        break;
      } else {
        fd = open(req.pathName, O_RDONLY);
        if (fd == -1) {
          int eno = errno;
          if(errno == ENOENT) {
            strncpy(buf, "open error\n", 10);
            // send err message
            if(write(STDOUT_FILENO, buf, 10) != 10) {
              perror("[parent] write");
              exit(1);
            }
            errno = eno;
            perror("open");
            wait(NULL);
            printf("[parent] exit\n");
            exit(EXIT_FAILURE);
          } else {
            // if other errror occurs, exit immediately
            perror("open");
            exit(1);
          }
        }
        
        // send response message
        fprintf(stdout, "[parent] requested by PID: %d\n", req.clientId);
        while(1) {
          if((numRead = read(fd, buf, BUF_SIZE)) < 0) {
            perror("read");
            exit(1);
          } else if (numRead == 0) {
            printf("[parent] EOF\n");
            if(close(fd) == -1) {
              perror("close");
              exit(1);
            }
            break;
          } else {
            memset(&res, 0, sizeof(res));
            res.data = malloc(numRead);
            res.mtype = RESP_DATA;
            res.data = buf;
            res.len = numRead;
            printf("[parent] send message, msg: %p\n", res.data);
            if(write(rpfd[1], &res, sizeof(res)) != sizeof(res)) {
                perror("[parent] write");
                exit(1);
            }
          }
        }
      }
    }
    // end message
    if(close(pfd[0]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    if(close(rpfd[1]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    wait(NULL);
    printf("[parent] exit\n");
    exit(EXIT_SUCCESS);
  }

}
