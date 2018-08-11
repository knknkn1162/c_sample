#define _DEFAULT_SOURCE
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static sigjmp_buf senv;
static volatile sig_atomic_t canJump = 0;
void doSth(void);

static void handler(int sig) {
  if(!canJump) {
    write(STDOUT_FILENO, "not yet set\n", 13);
    return;
  }
  siglongjmp(senv, 1);
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

  printf("calling sigsetjmp\n");
  
  if(sigsetjmp(senv, 1) == 0) {
    // for the first time, set senv
    canJump = 1;
  } else {
    doSth();
  }
  // TODO: do we need while loop?
  while(1) {
    pause();
  }
}

void doSth() {
  printf("do sth!\n");
}
