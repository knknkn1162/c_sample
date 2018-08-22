#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

#define PIPE_MAX 10

#define REQ_MSG_SIZE PATH_MAX
struct request {
  long clientId;
  char pathName[REQ_MSG_SIZE];
};
#define REQ_SIZE (sizeof(struct request))
#define RESP_SIZE 256

int main(int argc, char *argv[]) {
  
  int pfd[2], rpfd[PIPE_MAX][2];
  int i, j;
  pid_t pid;
  pid_t pid_table[PIPE_MAX];
  struct sigaction sa;
  
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

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDWAIT;
  sa.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &sa, 0);

  // send requests
  for(i = 1; i < argc; i++) {
    if((pid = fork()) < 0) {
      perror("fork");
      exit(1);
    } else if(pid == 0)  {
      struct request req;
      char buf[RESP_SIZE];
      int numRead;
      // child
      if(close(pfd[0]) == -1) {
        perror("[child] close read fd");
        exit(1);
      }
      

      // close all write rpfds
      // close rpfd read except the index `i`.
      for(j = 1; j < argc; j++) {
        if(close(rpfd[j-1][1]) == -1) {
          perror("[child] close");
          exit(1);
        }
        if(i == j) continue;
        if(close(rpfd[j-1][0]) == -1) {
          perror("[child] close");
          exit(1);
        }
      }

      // send request
      strncpy(req.pathName, argv[i], strlen(argv[i]));
      req.clientId = getpid();

      printf("[child(%d)] send request\n", i-1);
      if(write(pfd[1], &req, REQ_SIZE) != REQ_SIZE) {
        perror("[child] write");
        exit(1);
      }

      if(close(pfd[1]) == -1) {
        perror("[child] close write");
        exit(1);
      }

      // receive response
      printf("[child(%d)] wait response\n", i-1);
      while(1) {
        if((numRead = read(rpfd[i-1][0], buf, RESP_SIZE)) < 0) {
          perror("[child] read");
          continue;
        } else if(numRead == 0) {
          // doesn't work!!
          printf("[child] EOF\n");
          break;
        }
        printf("[child(%d)]> %s\n",i - 1, buf);
      }

      if(close(rpfd[i-1][0]) == -1) {
        perror("close");
        exit(1);
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
  char buf[RESP_SIZE];
  struct request req;
  int idx = -1;
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
    int serverPid;
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

    if((serverPid = fork()) < 0) {
      perror("fork");
      exit(1);
    } else if(serverPid > 0) {
      if(close(rpfd[idx][1]) == -1) {
        perror("[parent] close");
        exit(1);
      }
      printf("[parent] end reactor\n");
      continue;
    }

    // server clone
    serverPid = getpid();
    printf("[parent(%d)] requested by %ld (%d)\n", serverPid, req.clientId, idx);
    ifd = open(req.pathName, O_RDONLY);

    if(ifd == -1) {
      perror("open");
      continue;
    }
    

    // send response
    printf("[parent(%d)->child(%d)] send response\n", serverPid, idx);
    while(1) {
      if((numRead = read(ifd, buf, RESP_SIZE)) == -1) {
        perror("[parent] read");
        continue;
      } else if(numRead == 0) {
        printf("[parent] EOF\n");
        break;
      }
      if(write(rpfd[idx][1], buf, numRead) != numRead) {
        perror("child");
        break;
      }
    }
    if(close(ifd) == -1) {
      perror("close");
      exit(1);
    }

    if(close(rpfd[idx][1]) == -1) {
      perror("[parent] close");
      exit(1);
    }
    printf("[parent(%d)->child(%d)] exit\n", serverPid, idx);
    _exit(EXIT_SUCCESS);
  } // while end

  if(close(pfd[0]) == -1) {
    perror("[parent] close");
    exit(1);
  }

  printf("[parent] wait..\n");
  wait(NULL);
  printf("[parent] exit\n");
  exit(EXIT_SUCCESS);
}
