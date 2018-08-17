#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 100

int main(int argc, char *argv[]) {
  pid_t pid;

  if((pid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if(pid == 0) {
    char buf[BUF_SIZE];
    int numRead;
    // child
    while((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) >= 0) {
      printf("num: %d\n", numRead);
    }
    _exit(EXIT_SUCCESS);
  } else {
    //parent
    exit(EXIT_SUCCESS);
  }
}
