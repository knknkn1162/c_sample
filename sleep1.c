#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

void signal_handler(int no);

int main(void) {
  struct sigaction sa;
  int ret;

  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;

  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, 0);

  while(1) {
    ret = sleep(3600);
    // return EINTR=Interrupted system call with SIGINT even if setting SA_RESTART,
    // so `errno` == EINTR` condition is required
    if(ret != 0 && errno == EINTR) { continue; }
    break;
  }

  return 0;
}

void signal_handler(int no) {
  char *mes = "signal get\n";
  write(1, mes, strlen(mes));
}
