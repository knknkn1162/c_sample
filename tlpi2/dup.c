#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(void) {
  int fd = open("Dockerfile", O_RDONLY);
  int numRead, res;

  dup2(fd, STDOUT_FILENO);

  char buf[1024];
  numRead = read(STDOUT_FILENO, buf, 1024);

  fprintf(stderr, "> %s \n", buf);
  if(write(STDOUT_FILENO, buf, numRead) == -1) {
    if(errno == EBADF) {
      // 9 (Bad file descriptor)
      fprintf(stderr, "%d (%s)\n", errno, strerror(errno));
    }
  }


}
