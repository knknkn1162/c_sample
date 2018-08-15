#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void signal_handler(int sig) {}

int main(int argc, char *argv[]) {
  pid_t pid;
  struct sigaction sa;

  setbuf(stdout, NULL);

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = signal_handler;

  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if((pid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if(pid == 0) {
    if(argc > 1) {
      if(setpgid(0, 0) == -1) {
        perror("setpgid");
        exit(1);
      }
    }
  }

  printf("pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
  while(1) {
    pause();
    printf("%ld: \n", (long)getpid());
    return 0;
  }
}
