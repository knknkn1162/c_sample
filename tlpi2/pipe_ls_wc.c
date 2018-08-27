#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[]) {
  int pfd[2][2];
  int i, j;
  struct sigaction sa;

  sa.sa_flags = SA_NOCLDWAIT;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(pipe(pfd[0]) == -1) {
    perror("pipe");
    exit(1);
  }

  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      if(close(pfd[0][0]) == -1) {
        perror("close read");
        exit(1);
      }

      // STDOUT_FILENO -> pfd[1]
      if(pfd[0][1] != STDOUT_FILENO) {
        if(dup2(pfd[0][1], STDOUT_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }
        if(close(pfd[0][1]) == -1) {
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

  /* wait(NULL); */
  /* char buf[1024]; */
  /* int numRead = read(pfd[0][0], buf, 1024); */
  /* fprintf(stderr, "> %d %s\n", numRead, buf); */
  /* return 0; */


  if(pipe(pfd[1]) == -1) {
    perror("pipe");
    exit(1);
  }

  // generate next process
  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      if(close(pfd[1][0]) == -1) {
        perror("close read");
      }
      if(close(pfd[0][1]) == -1) {
        perror("close");
      }
      if(pfd[0][0] != STDIN_FILENO) {
        if(dup2(pfd[0][0], STDIN_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }
        if(close(pfd[0][0]) == -1) {
          perror("close");
          exit(1);
        }
      }

      fprintf(stderr, "ss\n");
      if(pfd[1][1] != STDOUT_FILENO) {
        if(dup2(pfd[1][1], STDOUT_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }

        if(close(pfd[1][1]) == -1) {
          perror("close");
        }
      }
      fprintf(stderr, "tt\n");

      execlp("grep", "grep", "a", (char*)NULL);
      perror("execlp grep");
      exit(1);
    default:
      for(i = 0; i < 2; i++) {
        if(close(pfd[0][i]) == -1) {
          perror("close");
          exit(1);
        }
      }
      break;
  }

  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      if(close(pfd[1][1]) == -1) {
        perror("close write");
      }

      fprintf(stderr, "3ss\n");
      if(pfd[1][0] != STDIN_FILENO) {
        if(dup2(pfd[1][0], STDIN_FILENO) == -1) {
          perror("dup2");
          exit(1);
        }
        if(close(pfd[1][0]) == -1) {
          perror("close");
          exit(1);
        }
      }
      fprintf(stderr, "3tt\n");

      execlp("wc", "wc", "-c", (char*)NULL);
      perror("execlp wc");
      exit(1);
    default:
      break;
  }

  // close
  for(j = 0; j < 2; j++) {
    if(close(pfd[1][j]) == -1) {
      perror("close read");
      exit(1);
    }
  }

  wait(NULL);

  return 0;
}
