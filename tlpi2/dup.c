#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(void) {
  int fd = open("test.txt", O_RDWR);

  if(fork() == 0) {
    dup2(fd, STDOUT_FILENO);

    if(close(fd) == -1) {
      perror("close");
      exit(1);
    }
    execlp("ls", "ls", (char*)NULL);
    exit(1);
  }

  if(fork() == 0) {
    dup2(fd, STDIN_FILENO);
    if(close(fd) == -1) {
      perror("exit");
    }
    execlp("wc", "wc", "-l", (char*)NULL);
    exit(1);
  }

  wait(NULL);
  exit(1);

}
