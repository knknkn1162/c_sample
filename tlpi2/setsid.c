#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char *argv[]) {

  printf("pid=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getpgrp(), (long)getsid(0));

  if(fork() != 0) {
    _exit(EXIT_SUCCESS);
  }

  if(setsid() == -1) {
    perror("setsid");
    exit(1);
  }

  printf("pid=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getpgrp(), (long)getsid(0));

  if(open("/dev/tty", O_RDWR) == -1) {
    perror("open /dev/null");
    exit(1);
  }

  exit(EXIT_SUCCESS);
}
