#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


static void handler(int sig) { printf("signal: %d(%d)\n", sig, getpid()); }

int main(void) {
  pid_t pid;

  if((pid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if(pid == 0) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if(sigaction(SIGHUP, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }

    sa.sa_flags = 0;
    if(sigaction(SIGCONT, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }
    if(sigaction(SIGTERM, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }

    printf("child: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
    sleep(120);
    printf("child exit\n");
    exit(EXIT_SUCCESS);
  } else {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    if(sigaction(SIGCHLD, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }

    printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
    sleep(30);
    printf("parent exit\n");
    exit(EXIT_SUCCESS);
  }
  return 0;
}
