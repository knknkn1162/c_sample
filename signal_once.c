#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> // sigaction
#include <errno.h>
#include <string.h>

void signal_handler(int no);
int STDIN = 0;
int STDOUT = 1;

int main() {
  char buf[256];
  int ret;
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = signal_handler;
  // exec handler only once.
  // while sleep, the action, Ctrl+C is stored.
  sa.sa_flags = SA_RESTART | SA_RESETHAND;
  // int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
  // or
  /* or more information about information
  sa.sa_sigaction = signal_handler;
  sa.sa_flags  = SA_SIGINFO;
  */
  if(sigaction(SIGINT, &sa, NULL) != 0) {
    fprintf(stderr, "sigint(Ctrl+h) error\n");
    exit(1);
  }

  if(sigaction(SIGHUP, &sa, NULL) != 0) {
    fprintf(stderr, "sighup error\n");
    exit(1);
  }

  while(1) {
    memset(buf, '\0', sizeof(buf));
    printf("read wait\n");
    ret = read(STDIN, buf, sizeof(buf));
    if(ret <= 0) {
      fprintf(stderr, "readerror %d %s\n", errno, strerror(errno));
      exit(1);
    }
    write(STDOUT, buf, ret);
  }
}

void signal_handler(int no) {
  char *mes1 = "signal get\n";
  char *mes2 = "signal end\n";
  write(1, mes1, strlen(mes1));
  sleep(2);
  write(1, mes2, strlen(mes2));
}
