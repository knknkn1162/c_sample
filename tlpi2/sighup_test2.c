#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


volatile sig_atomic_t sighupCatch = 0;
volatile sig_atomic_t sigtermCatch = 0;
volatile sig_atomic_t sigcontCatch = 0;
void term_handler(int sig) { sigtermCatch = 1; }
void hup_handler(int sig) { sighupCatch = 1; }
void cont_handler(int sig) { sigcontCatch = 1; }


int main(int argc, char *argv[]) {
  struct sigaction sa;
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = hup_handler;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  sa.sa_handler = term_handler;
  if(sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  sa.sa_handler = cont_handler;
  if(sigaction(SIGCONT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  printf("pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
  fflush(stdout);

  // To ensure that the process should be terminated
  while(1) {
    pause();
    if(sighupCatch || sigtermCatch || sigcontCatch) {
      printf("term: %d hup: %d cont: %d  pid=%ld, PPID=%ld, PGID=%ld, SID=%ld\n", sigtermCatch, sighupCatch, sigcontCatch, (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
      fflush(stdout);
    }
  }

  return 0;
}
