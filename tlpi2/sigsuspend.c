#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t gotSignal = 0;
static void handler(int sig) { gotSignal = 1; }

int main(int argc, char *argv[]) {
  sigset_t origMask, blockMask;
  struct sigaction sa;

  // we want to prevent signal interuption on initialization.
  // Otherwise, we accept to terminate process with SIGINT.
  sigemptyset(&blockMask);
  sigaddset(&blockMask, SIGINT);
  if(sigprocmask(SIG_SETMASK, &blockMask, &origMask) == -1) {
    perror("sigemptyset to prevent interuption");
    exit(1);
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
  }

  // initialization
  printf("init...\n");
  sleep(5);

  // We accept SIGINT only after the initialization.
  if(sigsuspend(&origMask) == -1) {
    perror("sigsuspend");
    exit(1);
  }

  // restore
  if(sigprocmask(SIG_SETMASK, &origMask, NULL) == -1) {
    perror("sigemptyset to restore");
  }

  return 0;

}
