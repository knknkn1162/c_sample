#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void signal_handler(int sig) {}

int main(int argc, char *argv[]) {
  pid_t pid;
  struct sigaction sa;

  printf("pid: %d\n", getpid());

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = signal_handler;

  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  sleep(10);
  printf("end\n");
  sleep(10);
  
  return 0;
}
