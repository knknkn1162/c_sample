#include <stdio.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {
  printf("sigstop\n");
}

int main(void) {
  int sig;
  sigset_t allSigs;

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
      case SIGTSTP:
        printf("sigtstp\n");
        break;
      case SIGSTOP:
        printf("sigstop\n");
        break;
      default:
        exit(EXIT_SUCCESS);
    }
  }
}
