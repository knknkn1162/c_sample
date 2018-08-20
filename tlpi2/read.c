#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define BUF_SIZE 100

volatile sig_atomic_t sigCatch = 0;
static void handler(int sig) { sigCatch = 1; }

int main(int argc, char* argv[]) {
  char buf[BUF_SIZE];
  int numRead;
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;
  if(sigaction(SIGTTIN, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    putchar('.');
  }

  return 0;
}
