#define _DEFAULT_SOURCE
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int cmdNum;

static void handler(int sig) {
  if(getpid() == getpgrp()) {
    fprintf(stderr, "Terminal FG process group:%ld\n", (long)tcgetpgrp(STDERR_FILENO));
  }
  fprintf(stderr, "Progress %ld(%d) received signal %d (%s)\n", (long)getpid(), cmdNum, sig, strsignal(sig));

  if(sig == SIGTSTP) {
    raise(SIGSTOP);
  }
}

int main(int argc, char *argv[]) {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  if(sigaction(SIGTSTP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  if(sigaction(SIGCONT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(isatty(STDIN_FILENO)) {
    fprintf(stderr, "Terminal FG progress group: %ld\n", (long)tcgetpgrp(STDIN_FILENO));
    fprintf(stderr, "Command PID PPID PGRP SID\n");
    cmdNum = 0;
  } else {
    if(read(STDIN_FILENO, &cmdNum, sizeof(cmdNum)) <= 0) {
      perror("read");
      exit(1);
    }
  }

  cmdNum++;
  fprintf(stderr, "%4d %ld %ld %ld %ld\n", cmdNum, (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  if(!isatty(STDOUT_FILENO)) {
    if(write(STDOUT_FILENO, &cmdNum, sizeof(cmdNum)) == -1) {
      perror("write");
      exit(1);
    }
  }

  while(1) {
    pause();
  }
}
