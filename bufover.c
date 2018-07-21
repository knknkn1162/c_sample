#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define BUF_SIZE 16

void signal_handler(int signum);

int main() {
  char* wrk = malloc(BUF_SIZE);
  struct sigaction sa;

  sa.sa_handler = signal_handler;
  // catch segmentation fault
  sigaction(SIGSEGV, &sa, 0);

  while(1) {
    printf("%p\n", wrk);
    *wrk++ = 0xcc;
  }

  return 0;
}

void signal_handler(int signum) {
  fprintf(stdout, "segmentation fault; sig: [%d]\n", signum);
  exit(1);
}
