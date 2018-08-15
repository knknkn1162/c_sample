#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void tstpHandler(int sig);

int main(int argc, char *argv[]) {
  struct sigaction sa;
  if(sigaction(SIGTSTP, NULL, &sa) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(sa.sa_handler != SIG_IGN) {
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = tstpHandler;
    if(sigaction(SIGTSTP, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }
  }

  while(1) {
    pause();
    printf("Main\n");
  }
}


static void tstpHandler(int sig) {
  sigset_t tstpMask, prevMask;
  int savedErrno;
  struct sigaction sa;

  savedErrno = errno;
  printf("Caught SIGTSTP\n");

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = SIG_DFL;
  if(sigaction(SIGTSTP, &sa, NULL) == -1) {
    perror("signal");
  }

  printf("before raise SIGTSTP\n");
  raise(SIGTSTP);
  printf("after raise SIGTSTP\n");
  /* unblock SIGTSTP, the pending SIGTSTP immediately suspends the program */
  sigemptyset(&tstpMask);
  sigaddset(&tstpMask, SIGTSTP);
  if(sigprocmask(SIG_UNBLOCK, &tstpMask, &prevMask) == -1) {
    perror("sigprocmask");
  }
  printf("sigprocmask SIG_UNBLOCK\n");

  if(sigprocmask(SIG_SETMASK, &prevMask, NULL) == -1) {
    perror("sigprocmask");
  }
  printf("sigprocmask SIG_BLOCK\n");

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = tstpHandler;
  if(sigaction(SIGTSTP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("Exiting SIGTSTP handler\n");

  errno = savedErrno;
}
