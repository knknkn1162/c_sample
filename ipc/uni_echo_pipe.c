#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 256

int main(int argc, char *argv[]) {
  
  int pfd[2];
  pid_t pid;

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  // file descriptors are duplicated on each processes.
  if((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    // child
    if(close(pfd[0]) == -1) {
      perror("[child] close read fd");
    }
    if(write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1])) {
      perror("[child] write");
      exit(1);
    }

    if(close(pfd[1]) == -1) {
      perror("[child] close write");
    }
    printf("child exit\n");
    _exit(EXIT_SUCCESS);
  } else {
    //parent
    int numRead;
    char buf[BUF_SIZE];
    if(close(pfd[1]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    // receive request
    while(1) {
      if((numRead = read(pfd[0], buf, BUF_SIZE))== -1) {
        perror("read");
      } else if (numRead == 0) {
        printf("read eof\n");
        break;
      }
      if(write(STDOUT_FILENO, buf, numRead) != numRead) {
        perror("child");
        exit(1);
      }
    }
    // end message
    write(STDOUT_FILENO, "\n", 1);
    if(close(pfd[0]) == -1) {
      perror("[parent] close");
      exit(1);
    }
    wait(NULL);
    printf("parent exit\n");
    exit(EXIT_SUCCESS);
  }

}
