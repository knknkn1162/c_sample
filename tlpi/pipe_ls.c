#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void errExit(const char* msg) {
  perror(msg);
  exit(1);
}

int main(void) {
  int pfd[2];

  if(pipe(pfd) == -1) { errExit("pipe"); }


  switch(fork()) {
    case -1:
      errExit("fork");
    case 0:
      if(close(pfd[0]) == -1) {
        errExit("close 1");
      }

      if(pfd[1] != STDOUT_FILENO) {
        if(dup2(pfd[1], STDOUT_FILENO) == -1) {
          errExit("dup2 1");
        }
        if(close(pfd[1]) == -1) {
          errExit("close pfd[1]");
        }


        execlp("ls", "ls", (char*)NULL);
        errExit("execlp ls");
      }

    default:
      break;
  }

  switch(fork()) {
    case -1:
      errExit("fork");
    case 0:
      if(close(pfd[1]) == -1) {
        errExit("close pfd[1] 3");
      }

      if(pfd[0] != STDIN_FILENO) {
        if(dup2(pfd[0], STDIN_FILENO) == -1) {
          errExit("dup2 2");
        }
        if(close(pfd[0]) == -1) {
          errExit("close 4");
        }
      }

      execlp("wc", "wc", (char*)NULL);
      errExit("execlp wc");
  }

  if(close(pfd[0]) == -1) {
    errExit("close 5");
  }
  if(close(pfd[1]) == -1) {
    errExit("close 6");
  }

  if(wait(NULL) == -1) {
    errExit("wait 1");
  }
  if(wait(NULL) == -1) {
    errExit("wait 2");
  }

  exit(EXIT_SUCCESS);
}
