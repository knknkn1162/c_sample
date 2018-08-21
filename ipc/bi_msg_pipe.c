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
#define PIPE_MAX 10

int main(int argc, char *argv[]) {
  
  int pfd[2], rpfd[PIPE_MAX][2];
  int i;
  pid_t pid;
  pid_t pid_table[PIPE_MAX];
  
  if(argc > PATH_MAX) {
    fprintf(stderr, "the # of argument is too numerous!\n");
    exit(1);
  }

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  for(i = 1; i < argc; i++) {
    if(pipe(rpfd[i-1]) == -1) {
      perror("pipe");
      exit(1);
    }
  }

  for(i = 1; i < argc; i++) {
    if((pid = fork()) < 0) {
      perror("fork");
      exit(1);
    } else if(pid == 0)  {
      struct request req;
      struct response resp;
      int numRead;
      int flag = 1;
      // child
      if(close(pfd[0]) == -1) {
        perror("[child] close read fd");
        exit(1);
      }
      
      if(close(rpfd[i-1][1]) == -1) {
        perror("[child] close");
        exit(1);
      }

      // send request
      strncpy(req.pathName, argv[i], strlen(argv[i]));
      req.clientId = getpid();

      printf("[child(%d)] send request\n", i-1);
      if(write(pfd[1], &req, REQ_SIZE) != REQ_SIZE) {
        perror("[child] write");
        exit(1);
      }

      // receive response
      printf("[child] receive response\n");
      while(flag) {
        if((numRead = read(rpfd[i-1][0], &resp, RESP_SIZE)) < 0) {
          perror("[child] read");
          continue;
        } else if(numRead == 0) {
          printf("[child] EOF\n");
          break;
        }
        switch(resp.mtype) {
          case RESP_FAILURE:
            fprintf(stderr, "[child] response error: %s\n", resp.message);
            flag = 0;
            break;
          case RESP_END:
            printf("[child(%d)] END response\n", i - 1);
            // The descriptor rpfd remains open, so we can't detect EOF. 
            // We can't break loop withou switching on the flag explicitly.
            flag = 0;
            break;
          case RESP_DATA:
            printf("[child(%d)]> %s\n",i - 1, resp.message);
            break;
        }
      }

      if(close(pfd[1]) == -1) {
        perror("[child] close write");
      }
      printf("[child(%d)] exit\n", i - 1);
      _exit(EXIT_SUCCESS);
    } else {
      pid_table[i-1] = pid;
    }
  } // end for

  /* parent process */
  int numRead;
  int ifd;
  char buf[BUF_SIZE];
  struct request req;
  struct response resp;
  int idx = -1;
  int cnt = argc - 1; // the number of child processes
  if(close(pfd[1]) == -1) {
    perror("[parent] close");
    exit(1);
  }

  for(i = 1; i < argc; i++) {
    if(close(rpfd[i-1][0]) == -1) {
      perror("[parent] close");
      exit(1);
    }
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
      // The write descriptor pfd[1] all closes..
      printf("[parent] read eof\n");
      break;
    }

    // search rpfd idx
    for(i = 1; i < argc; i++) {
      if(pid_table[i-1] == req.clientId) {
        idx = i - 1;
        break;
      }
    }
    if(idx == -1) {
      perror("clientId");
      exit(1);
    }

    printf("[parent] requested by %ld (%d)\n", req.clientId, idx);
    ifd = open(req.pathName, O_RDONLY);
    if(ifd == -1) {
      int savedErrno;
      resp.mtype = RESP_FAILURE;
      strncpy(resp.message, strerror(errno), strlen(strerror(errno) + 1));
      savedErrno = errno;
      if(write(rpfd[idx][1], &resp, sizeof(resp)) != sizeof(resp)) {
          perror("write");
          break;
      }
      errno = savedErrno;
      perror("open");
      continue;
    }
    

    // send response
    printf("[parent->child(%d)] send response\n", idx);
    while(1) {
      if((numRead = read(ifd, &buf, RESP_MSG_SIZE)) == -1) {
        perror("[parent] read");
        continue;
      } else if(numRead == 0) {
        // end msg
        printf("[parent] EOF\n");
        resp.mtype = RESP_END;
        write(rpfd[idx][1], &resp, sizeof(resp));
        break;
      }
      resp.mtype = RESP_DATA;
      strncpy(resp.message, buf, numRead);
      if(write(rpfd[idx][1], &resp, RESP_SIZE) != RESP_SIZE) {
        perror("child");
        break;
      }
    }
    if(close(rpfd[idx][1]) == -1) {
      perror("[parent] close");
      exit(1);
    }
    printf("[parent] end reactor\n");
  } // while end

  if(close(pfd[0]) == -1) {
    perror("[parent] close");
    exit(1);
  }

  // All child processes exit before this process exits..
  while(1) {
    if(waitpid(-1,  NULL, 0) == -1) {
      perror("waitpid");
    }
    printf("[parent] wait for child .. %d\n", cnt);
    if(--cnt == 0) break;
  }

  printf("[parent] exit\n");
  exit(EXIT_SUCCESS);
}
