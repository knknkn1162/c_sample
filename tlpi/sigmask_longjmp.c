#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t canJump = 0;

static sigjmp_buf senv;

static void signal_handler(int sig) {
  printf("received %s\n", strsignal(sig));
  if(!canJump) {
    printf("env buffer not yet set\n");
  }
  siglongjmp(senv, 1);
}

int main(void) {
  struct sigaction sa;
  printf("startup\n");

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = signal_handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(sigsetjmp(senv, 1) == 0) {
    printf("jump\n");
    canJump = 1;
  } else {
    printf("after\n");
  }
  while(1) {
    pause();
  }

  exit(EXIT_SUCCESS);
}
