#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


static void signalHandler(int sig) {
  int x = 1;
  printf("stack %10p\n", (void*)&x);
  _exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  struct sigaction sa;

  printf("heap is at %10p\n", (char*)sbrk(0));

  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  while(1) {
    pause();
  }

  return 0;
}

