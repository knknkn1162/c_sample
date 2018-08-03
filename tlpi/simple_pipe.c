#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 10

int main(int argc, char* argv[]) {
  int pfd[2];
  char buf[BUF_SIZE];
  ssize_t numRead;

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  switch(fork()) {
    case -1:
      perror("fork");
      break;
    case 0:
      // close write
      if(close(pfd[1] == -1) == -1) {
        perror("close");
        exit(1);
      }

      while(1) {
        numRead = read(pfd[0], buf, BUF_SIZE);
        if(numRead == -1) {
          perror("read");
        }
        if(numRead == 0) { break; }
        if(write(STDOUT_FILENO, buf, numRead) != numRead) {
          perror("write file");
          exit(1);
        }
      }
      write(STDOUT_FILENO, "\n", 1);
      if(close(pfd[0]) == -1) {
        perror("close");
      }

      _exit(EXIT_SUCCESS);
    default:
      if(close(pfd[0]) == -1) { perror("close0"); exit(1); }
      while(1) {
        if(write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1])) {
          perror("parent write");
          exit(1);
        }
        sleep(1);
      }
      if(close(pfd[1]) == -1) {
        perror("close");
        exit(1);
      }

      wait(NULL);
      exit(EXIT_SUCCESS);
  }

  return 0;
}
