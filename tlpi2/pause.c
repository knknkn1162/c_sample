#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

void handler(int sig) {printf("sigtstp");}

int main(int argc, char *argv[]) {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGSTOP, &sa, 0) == -1) {
    perror("sigaction");
    exit(1);
  }
  while(1) {
    sleep(1);
  }
  return 0;
}
