#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>


void signal_handler(int sig);
static volatile sig_atomic_t gotSignal = 0;

int main(int argc, char *argv[]) {
  alarm(5);

  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = signal_handler;

  if(sigaction(SIGALRM, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    if(gotSignal) {
      printf("got sigalarm\n");
      exit(EXIT_SUCCESS);
    }
  }
}

void signal_handler(int sig) {
  gotSignal = 1;
}
