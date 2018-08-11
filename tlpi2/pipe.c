#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "req_res.h"

int main(int argc, char *argv[]) {
  int pfd[2]; // client-> server
  int respfd[2]; //server -> client
  char buf[BUF_SIZE];
  int numRead;
  int i;
  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  if(pipe(respfd) == -1) {
    perror("pipe respfd");
    exit(1);
  }
  for(i = 1; i < argc; i++) {
    switch(fork()) {
      case -1:
        perror("fork");
        exit(1);
      case 0:
        // read end is unused
        if(close(pfd[0]) == -1) {
          perror("exit");
          exit(1);
        }
        if(close(respfd[1]) == -1) {
          perror("exit");
          exit(1);
        }

        // write server
        if(write(pfd[1], argv[i], strlen(argv[i])) == -1) {
          perror("write");
          exit(1);
        }
        while((numRead = read(respfd[0], buf, BUF_SIZE)) > 0) {
          write(STDOUT_FILENO, buf, numRead);
        }
        printf("client %d end\n", i);
        _exit(EXIT_SUCCESS);
      default:
        break;
    }
  }

  // parent[server]
  // write end is unused
  if(close(pfd[1]) == -1) {
    perror("close");
    exit(1);
  }

  while((numRead = read(pfd[0], buf, BUF_SIZE)) > 0) {
    write(respfd[1], buf, numRead);
    printf("response to client\n");
  }

  return 0;
}
