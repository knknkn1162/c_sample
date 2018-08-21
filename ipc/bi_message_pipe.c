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
  
  // for request
  int pfd[2];
  // for response
  int rpfd[2];
  pid_t pid;

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
    struct response resp;
    int numRead;
    int flag = 1;
    // child
    if(close(pfd[0]) == -1) {
      perror("[child] close read fd");
      exit(1);
    }
    
    // remain open
    if(close(rpfd[1]) == -1) {
      perror("[child] close");
      exit(1);
    }

    // send request
    strncpy(req.pathName, argv[1], strlen(argv[1]));
    req.clientId = getpid();

    printf("[child] send request\n");
    if(write(pfd[1], &req, REQ_SIZE) != REQ_SIZE) {
      perror("[child] write");
      exit(1);
    }

    // receive response
    printf("[child] receive response\n");
    while(flag) {
      if((numRead = read(rpfd[0], &resp, RESP_SIZE)) < 0) {
        perror("[child] read");
        continue;
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
          printf("> %s\n", resp.message);
          break;
      }
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
      memset(&resp, 0, sizeof(resp));
      // receive request
      printf("[parent] receive reuqest\n");
      if((numRead = read(pfd[0], &req, REQ_SIZE))== -1) {
        perror("[parent] read");
        continue;
      } else if (numRead == 0) {
        // If The descriptor pfd[1] (for writer) all closes..
        printf("[parent] read eof\n");
        break;
      }
      printf("[parent] requested by %ld\n", req.clientId);
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
  }

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
