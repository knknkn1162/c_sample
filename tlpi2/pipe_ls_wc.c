#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[]) {
  int pfd[2];
  int i;
  struct sigaction sa;

  sa.sa_flags = SA_NOCLDWAIT;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      if(close(pfd[0]) == -1) {
        perror("close read");
        exit(1);
      }

      // STDOUT_FILENO -> pfd[1]
      if(pfd[1] != STDOUT_FILENO) {
        if(dup2(pfd[1], STDOUT_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }
        if(close(pfd[1]) == -1) {
          perror("close 2");
        }
      }

      execlp("ls", "ls", (char*)NULL);
      perror("execlp ls");
      exit(1);

    default:
      // To create next child
      break;
  }

  // generate next process
  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      if(close(pfd[1]) == -1) {
        perror("close write");
      }

      if(pfd[0] != STDIN_FILENO) {
        if(dup2(pfd[0], STDIN_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }
        if(close(pfd[0]) == -1) {
          perror("close");
          exit(1);
        }
      }

      execlp("wc", "c", "-l", (char*)NULL);
      perror("execlp wc");
      exit(1);
    default:
      break;
  }

  for(i = 0; i < 2; i++) {
    if(close(pfd[i]) == -1) {
      perror("close");
      exit(1);
    }
  }

  wait(NULL);

  return 0;
}
