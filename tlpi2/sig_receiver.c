#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static void handler(int sig);

int main(int argc, char *argv[]) {
  sigset_t blockingMask, pendingMask, oldBlockingMask;
  struct sigaction sa;

  sa.sa_handler = handler;
  sa.sa_flags = SA_RESTART;
  // sa_mask specifies a mask of signals which should be blocked during execution of the signal handler.
  sigemptyset(&sa.sa_mask);
  // int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
  sigaction(SIGINT, &sa, NULL);


  sigemptyset(&blockingMask);
  sigaddset(&blockingMask, SIGHUP);
  sigaddset(&blockingMask, SIGQUIT);
  sigaddset(&blockingMask, SIGINT);

  while(1) {
    sigprocmask(SIG_SETMASK, &blockingMask, &oldBlockingMask );
    printf("wait...\n");
    sleep(5);
    if(sigpending(&pendingMask) == -1) {
      perror("sigpending");
      exit(1);
    }
    printf("set sigpending\n");

    if(sigismember(&pendingMask, SIGQUIT)) {
      printf("sigquit!\n");
    }
    if(sigismember(&pendingMask, SIGINT)) {
      printf("sigint!\n");
    }
    sigprocmask(SIG_SETMASK, &oldBlockingMask, NULL);
  }
}


static void handler(int sig) {
  sigset_t pendingMask;
  write(STDOUT_FILENO, "call handler\n", 13);
}
