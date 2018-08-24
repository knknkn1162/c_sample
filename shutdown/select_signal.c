#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h> // MAX
#include <sys/signalfd.h>
#include <errno.h>

volatile sig_atomic_t sigFlag = 0;
void signal_handler(int sig) { sigFlag = 1; }
long doCalc(long num);
void doShutdown(void);

#define BUF_SIZE 100

int main(void) {
  struct sigaction sa;
  fd_set fds;
  int maxfd;
  int flags;
  char buf[BUF_SIZE];

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = signal_handler;
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  maxfd = STDIN_FILENO + 1; // always 1.

  while (1) {
    long num;
    siginfo_t info;
    int res = select(maxfd, &fds, NULL, NULL, NULL);
    printf("select: %d\n", res);
    if(res == -1) {
      if(sigFlag == 1 && errno == EINTR) {
        fprintf(stderr, "signal caught!\n");
        doShutdown();
        exit(1);
      }
      perror("select");
      exit(1);
    }

    if(FD_ISSET(STDIN_FILENO, &fds)) {
      if(fgets(buf, BUF_SIZE, stdin) == NULL) {
        perror("fgets");
        exit(1);
      }

      if(strlen(buf) > 20) {
        fprintf(stderr, "> [error] too big!\n");
        continue;
      }
      if(buf[0] == '\n') {
        continue;
      }
      
      num = atol(buf);
      printf("> %ld -> %ld\n", num, doCalc(num));
    }
  }
}

long doCalc(long num) {
  return num*2;
}

void doShutdown(void) {
  sigset_t mask, prevMask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK, &mask, &prevMask);
  printf("graceful shutdown..\n");
  // go shutdown..
  sleep(2);
  sigprocmask(SIG_SETMASK, &prevMask, 0);
}
