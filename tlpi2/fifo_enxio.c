#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char *argv[]) {
  int wfd;
  char *fifo = "/tmp/test_fifo";

  umask(0);
  if(mkfifo(fifo, O_CREAT | S_IRUSR | S_IWUSR) == -1) {
    if(errno != EEXIST) {
      perror("mkfifo");
      exit(1);
    }
  }

  /* rfd = open(fifo, O_RDONLY | O_NONBLOCK); */

  /* if(rfd == -1) { */
    /* perror("open"); */
    /* exit(1); */
  /* } */
  wfd = open(fifo, O_WRONLY | O_NONBLOCK);
  if(wfd == -1) {
    // No such device or address
    if(errno == ENXIO) {
      perror("write");
      exit(1);
    }
    exit(1);
  }

  if(write(wfd, "ddd", 4) == -1) {
    fprintf(stderr, "%d (%s)\n", errno, strerror(errno));
    if(errno == ENXIO) {
      perror("write");
      exit(1);
    }
  }
  printf("dd\n");
}
