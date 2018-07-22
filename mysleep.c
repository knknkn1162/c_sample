#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


void signal_handler(int num);

int main(void) {

  struct sigaction sa;

  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGALRM, &sa, NULL) != 0) {
    fprintf(stderr, "sigaction error\n");
    exit(1);
  }

  while(1) {
    alarm(1);
    pause();
  }
}

void signal_handler(int num) {
  write(1, ".", 2);
}
