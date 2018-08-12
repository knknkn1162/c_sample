#define _DEFAULT_SOURCE
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int sig;
  sigset_t allSigs;

  sigfillset(&allSigs);
  if(sigprocmask(SIG_SETMASK, &allSigs, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  while(1) {
    if(sigwait(&allSigs, &sig) != 0) {
      perror("sigwait");
    }
    switch(sig) {
      case SIGINT:
        printf("sigint\n");
        break;
      case SIGTERM:
        printf("sigterm\n");
        break;
      default:
        exit(EXIT_SUCCESS);
    }
  }
}
