#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


volatile sig_atomic_t sigCatch = 0;
void handler(int sig) { sigCatch = 1; }


int main(int argc, char *argv[]) {
  struct sigaction sa;
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  printf("pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  // To ensure that the process should be terminated
  alarm(60);
  while(1) {
    sleep(30);
    if(sigCatch) {
      printf("pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
      fflush(stdout);
      exit(EXIT_SUCCESS);
    }
    sigCatch = 0;
  }

  return 0;
}
