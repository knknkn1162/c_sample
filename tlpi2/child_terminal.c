#define _GNU_SOURCE
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define BUF_SIZE 256
#define MAX_SNAME 100

int ttySetRaw(int fd, struct termios *prevTermios);

// INCORRECT CODE!
int main(void) {
  pid_t childPid;
  struct winsize ws;
  struct termios ttyOrig;

  char* buf = ctermid(NULL); // usually /dev/tty
  // save termios settings
  tcgetattr(STDIN_FILENO, &ttyOrig);
  // save window size settings.
  ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

  printf("pid=%ld, PPID=%ld, PGID=%ld, SID=%ld, tty: %s\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0), ctermid(NULL));

  if((childPid = fork()) == 0) {
    int newFd;
    setsid();
    printf("[child, after setsid] pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
    newFd = open(buf, O_RDWR);
    if(newFd == -1) {
      perror("child open");
      exit(1);
    }

    if(ioctl(newFd, TIOCSCTTY, 0) == -1) {
      perror("ioctl");
      exit(1);
    }

    // reflect termios settings on the slaveFd
    tcsetattr(newFd, TCSANOW, &ttyOrig);

    // reflect window size settings on the slaveFd
    ioctl(newFd, TIOCSWINSZ, &ws);

    // 2.c) Use dup to duplicate the file descriptor for the slave device on standard input, output and error
    dup2(newFd, STDIN_FILENO);
    dup2(newFd, STDOUT_FILENO);
    dup2(newFd, STDERR_FILENO);

    printf("[child, before execlp] pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
    pause();
  } else {
    wait(NULL);
  }
}
