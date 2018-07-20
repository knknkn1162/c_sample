#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

void signal_handler(int no);

/*
^Csignal get
^Zsignal end

[2]+  Stopped                 ./a.out
 */
int main(void) {
  char buf[256];
  int ret;
  struct sigaction sa;
  // void     (*sa_handler)(int);
  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  /* prevent SIGTSTP while executing singal_handler callback function */
  sigaddset(&sa.sa_mask, SIGTSTP);
  sigaction(SIGINT, &sa, 0);

  while(1) {
    printf("read wait.\n");
    ret = read(0, buf, sizeof(buf));
    write(1, buf, ret);
  }

  return 0;
}


void signal_handler(int no) {
  char *mes1 = "signal get\n";
  char *mes2 = "signal end\n";
  write(1, mes1, strlen(mes1));
  sleep(5);
  write(1, mes2, strlen(mes2));
}
