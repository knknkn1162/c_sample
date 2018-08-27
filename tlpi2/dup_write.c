#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(void) {
  int fd = open("test.txt", O_WRONLY);
  int numRead;

  dup2(fd, STDOUT_FILENO);

  char buf[1024] = "ddsf\ndfdf\nzz";
  numRead = write(STDOUT_FILENO, buf, strlen(buf));

  //fprintf(stderr, "> %s \n", buf);
  /*
  if(write(STDOUT_FILENO, buf, numRead) == -1) {
    if(errno == EBADF) {
      // 9 (Bad file descriptor)
      fprintf(stderr, "%d (%s)\n", errno, strerror(errno));
    }
  }*/

  if(close(fd) == -1) {
    perror("close");
    exit(1);
  }
  fprintf(stderr, "close fd\n");

  // still fd
  memcpy(buf, "ao\n", 4);
  fprintf(stdout, buf, numRead);

}
