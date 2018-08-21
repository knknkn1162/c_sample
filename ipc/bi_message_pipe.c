#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "msg.h"

#define BUF_SIZE 256

volatile sig_atomic_t sigCatch = 0;
void sigint_handler(int sig) { sigCatch = 1; }

int main(int argc, char *argv[]) {
  
  // for request
  int pfd[2];
  // for response
  int rpfd[2];
  struct sigaction sa;
  pid_t pid;

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  if(pipe(rpfd) == -1) {
    perror("pipe");
    exit(1);
  }


  //inherit signal dispositions & sigmask after fork()
  // If the settings was implement by child process only, the parent process might exit first.
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // file descriptors are duplicated on each processes.
  if((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    /* child process */
    struct request req;
    struct response resp;
    int num;

    if(close(pfd[0]) == -1) {
      perror("[child] close read fd");
      exit(1);
    }
    
    if(close(rpfd[1]) == -1) {
      perror("[child] close");
      exit(1);
    }

    // send request
    while(1) {
      if(sigCatch) {
        printf("[child] catch SIGINT. Abort!\n");
        break;
      }
      int flag = 1;
      printf("[child] ready..\n");
      fgets( req.pathName, REQ_MSG_SIZE, stdin);
      if(errno == EINTR) {
        continue;
      }
      num = strlen(req.pathName);
      req.pathName[num-1] = '\0'; // replace '\n' with '\0'

      req.clientId = getpid();

      printf("[child] send request\n");
      if(write(pfd[1], &req, REQ_SIZE) != REQ_SIZE) {
        perror("[child] write");
        exit(1);
      }
      printf("[child] %s\n", req.pathName);

      // receive response
      printf("[child] receive response\n");
      while(flag) {
        if((num = read(rpfd[0], &resp, RESP_SIZE)) < 0) {
          perror("[child] read");
          continue;
        } else if(num == 0) {
          printf("[child] EOF\n");
          break;
        }
        switch(resp.mtype) {
          case RESP_FAILURE:
            fprintf(stderr, "[child] response error: %s\n", resp.message);
            exit(1);
          case RESP_END:
            printf("[child] END response\n");
            // The descriptor rpfd remains open, so we can't detect EOF.
            // We can't break loop withou switching on the flag explicitly.
            flag = 0;
            break;
          case RESP_DATA:
            printf("[child] > %s\n", resp.message);
            break;
        }
      }
      printf("[child] end reactor\n");
    }

    if(close(pfd[1]) == -1) {
      perror("[child] close write");
    }
    printf("[child] exit\n");
    _exit(EXIT_SUCCESS);
  } else {
    //parent
    int numRead;
    int ifd;
    char buf[BUF_SIZE];
    struct request req;
    struct response resp;
    // In case of not breaking the below reactor, unclose pipe file descriptor for writer.
    /* if(close(pfd[1]) == -1) { */
      /* perror("[parent] close"); */
      /* exit(1); */
    /* } */

    if(close(rpfd[0]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    // reactor
    while(1) {
      if(sigCatch) {
        break;
      }
      memset(&resp, 0, sizeof(resp));
      // receive request
      printf("[parent] ready..\n");
      if((numRead = read(pfd[0], &req, REQ_SIZE))== -1) {
        // when signal is caught..
        if(errno == EINTR) {
          printf("[parent] interrupted by SIGINT\n");
        } else {
          perror("[parent] read");
        }
        continue;
      } else if (numRead == 0) {
        // If The descriptor pfd[1] (for writer) all closes..
        printf("[parent] read eof\n");
        break;
      }
      printf("[parent] receive reuqest by %ld\n", req.clientId);
      ifd = open(req.pathName, O_RDONLY);
      if(ifd == -1) {
        int savedErrno;
        resp.mtype = RESP_FAILURE;
        strncpy(resp.message, strerror(errno), strlen(strerror(errno) + 1));
        savedErrno = errno;
        if(write(rpfd[1], &resp, sizeof(resp)) != sizeof(resp)) {
            perror("write");
            break;
        }
        errno = savedErrno;
        perror("open");
        continue;
      }
      

      // send response
      printf("[parent] send response\n");
      while(1) {
        if((numRead = read(ifd, &buf, RESP_MSG_SIZE)) == -1) {
          perror("[parent] read");
          continue;
        } else if(numRead == 0) {
          // end msg
          printf("[parent] EOF\n");
          resp.mtype = RESP_END;
          write(rpfd[1], &resp, sizeof(resp));
          break;
        }
        resp.mtype = RESP_DATA;
        strncpy(resp.message, buf, numRead);
        if(write(rpfd[1], &resp, RESP_SIZE) != RESP_SIZE) {
          perror("child");
          break;
        }
      }
      printf("[parent] end reactor\n");
    } // while end
  } // fork end

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
