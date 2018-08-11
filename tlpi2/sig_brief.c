#include <signal.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t gotSignal = 0;
static void handler(int sig) {
  gotSignal = 1;
}
void doSth();

int main(void) {

  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    if(gotSignal) {
      doSth();
      gotSignal = 0;
    }
  }
}

void doSth() {
  printf("got it!\n");
}
