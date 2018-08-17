#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static void handler(int sig) { printf("caught %d\n", sig);}

int main(int argc, char *argv[]) {
  pid_t childPid;
  struct sigaction sa;

  setbuf(stdout, NULL);
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if((childPid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if (childPid == 0) {
    if(argc > 1) {
      if(setpgid(0, 0) == -1) {
        perror("setpgid");
        exit(1);
      }
    }
  }

  printf("PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  pause();
  printf("%ld: caught SIGHUP\n", (long)getpid());
  return 0;
}
