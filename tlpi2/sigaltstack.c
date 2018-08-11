#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


static void signalHandler(int sig) {
  int x = 1;
  printf("signal: %d %s\n", sig, strsignal(sig));
  printf("stack %10p\n", (void*)&x);
  _exit(EXIT_FAILURE);
}

static void overflowStack(int callNum);

int main(int argc, char *argv[]) {
  stack_t sigstack;
  struct sigaction sa;

  sigstack.ss_sp = malloc(SIGSTKSZ);
  if(sigstack.ss_sp == NULL) {
    perror("malloc");
  }
  sigstack.ss_size = SIGSTKSZ;
  sigstack.ss_flags = 0;
  if(sigaltstack(&sigstack, NULL) == -1) {
    perror("sigstack");
    exit(1);
  }

  printf("%d Alternate stack is at %10p-%p(%ld)\n", SIGSTKSZ, sigstack.ss_sp, (char*)sbrk(0)-1, ((char*)sbrk(0)-1-(char*)sigstack.ss_sp));

  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_ONSTACK;

  if(sigaction(SIGSEGV, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  overflowStack(1);
}

static void overflowStack(int callNum) {
  char a[100000];

  printf("call %d %10p\n", callNum, &a[0]);
  overflowStack(callNum+1);
}
