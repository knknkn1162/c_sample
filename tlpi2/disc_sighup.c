#define _DEFAULT_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

static void handler(int sig) {
  fprintf(stderr, "PID %ld: caught signal %2d (%s)\n", (long)getpid(), sig, strsignal(sig));
}

int main(int argc, char *argv[]) {
  pid_t parentPid, childPid;
  int j;
  struct sigaction sa;

  setbuf(stdout, NULL);
  parentPid = getpid();
  fprintf(stderr, "PPID: %ld\n", (long)parentPid);
  fprintf(stderr, "Foreground PGID: %ld\n", (long)tcgetpgrp(STDIN_FILENO));

  for(j = 1; j < argc; j++) {
    childPid = fork();
    if(childPid == -1) {
      perror("fork");
      exit(1);
    }

    if(childPid == 0) {
      fprintf(stderr, "pid: %ld spawn\n", (long)getpid());
      if(argv[j][0] == 'd') {
        if(setpgid(0, 0) == -1) {
          perror("setpgid");
          exit(1);
        }
      }

      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      sa.sa_handler = handler;
      if(sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
      }
      fprintf(stderr, "[%d]pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", j, (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
      break;
    }
  }

  alarm(60);
  while(1) {
    pause();
  }
}
