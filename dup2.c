#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#

int main(void) {
  int fd;
  if((fd = open("./Dockerfile", O_RDONLY)) < 0) {
    perror("error");
    close(fd);
    exit(1);
  } else {
    close(0);
    dup2(fd, 0);
    close(fd);

    execlp("wc", "wc", NULL);
  }
}
