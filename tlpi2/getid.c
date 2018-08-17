#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>


int main(void) {
  char* buf = ctermid(NULL); // usually /dev/tty
  int fd = open(buf, O_RDWR);
  ioctl(fd, TIOCNOTTY);
  close(fd);
  // fd = open(buf, O_RDWR); // No such device or address
  if(fd == -1) {
    perror("open");
    exit(1);
  }
  printf("tcgetsid %d(ioctl: %d)\n", tcgetsid(STDIN_FILENO), ioctl(STDIN_FILENO, TIOCGSID));
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  pause();

  return 0;
}
