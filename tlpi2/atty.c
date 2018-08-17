#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  printf("pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
  if(isatty(STDIN_FILENO)) {
    printf("%d\n", tcgetpgrp(STDIN_FILENO));
  }

  return 0;
}
