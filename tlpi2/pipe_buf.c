#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#define BUF_SIZE 1000000

int main(int argc, char *argv[]) {
  mkfifo("/tmp/server", O_CREAT | S_IRUSR | S_IWUSR);

  int rfd = open("/tmp/server", O_RDONLY | O_NONBLOCK);
  int wfd = open("/tmp/server", O_WRONLY | O_NONBLOCK);

  char s[1000000];
  memset(s, 'a', 1000000);

  int flags = fcntl(wfd, F_GETFL);
  flags = O_WRONLY;
  fcntl(wfd, F_SETFL, flags);

  flags = fcntl(rfd, F_GETFL);
  flags = O_RDONLY;
  fcntl(rfd, F_SETFL, flags);

  if(fork() == 0) {
    while(1) {
      char buf[BUF_SIZE];
      int numRead = read(rfd, buf, BUF_SIZE);
      printf("size: %d\n", numRead);
      //write(STDOUT_FILENO, buf, numRead);
    }
  } else {
    close(rfd);
    if(write(wfd, s, 1000000) == -1) {
      perror("write");
      exit(1);
    }
    close(wfd);
    printf("done\n");
  }

  wait(NULL);
  return 0;
}
