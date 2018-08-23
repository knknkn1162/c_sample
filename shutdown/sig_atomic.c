#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 100

volatile sig_atomic_t sigFlag = 0;
void signal_handler(int sig) { sigFlag = 1; }
long doCalc(long a);
void doShutdown(void);

int main(int argc, char *argv[]) {
  char buf[BUF_SIZE];
  long num;
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = signal_handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  
  while(1) {
    if(sigFlag) {
      // do shutdown
      doShutdown();
      break;
    }
    // Strictly speaking, when the signal is caught, we don't know whether the function restarts or fails with error.
    // See also `Interruption of system calls and library functions by signal handlers` in `man 7 signal`.
    // But we assume that fgets use read syscall internally and fails with EINTR error, which is probably true.
    if(fgets(buf, BUF_SIZE, stdin) == NULL) {
      if(sigFlag == 1 && errno == EINTR) {
        printf("shutdown..\n");
        // do shutdown
        doShutdown();
        break;
      }
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

  return 0;
}


void doShutdown(void) {
  sigset_t mask, prevMask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK, &mask, &prevMask);
  // go shutdown..
  printf("graceful shutdown..\n");
  sleep(2);
  sigprocmask(SIG_SETMASK, &prevMask, 0);
}

long doCalc(long num) {
  return num*2;
}
