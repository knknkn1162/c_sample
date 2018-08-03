#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static void handler(int sig);

#define SYNC_SIG SIGUSR1

int main(void) {
  pid_t childPid;
  sigset_t blockMask, origMask, emptyMask;
  struct sigaction sa;

  setbuf(stdout, NULL);

  sigemptyset(&blockMask);
  sigaddset(&blockMask, SYNC_SIG);
  if(sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handler;
  if(sigaction(SYNC_SIG, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  switch(childPid = fork()) {
    case -1:
      exit(1);
    case 0:
      printf("PID: %d\n", getpid());
      sleep(2);

      kill(getppid(), SYNC_SIG);
      _exit(EXIT_SUCCESS);

    default:
      printf("parent pid: %d\n", getpid());
      sigemptyset(&emptyMask);
      if(sigsuspend(&emptyMask) == -1 && errno != EINTR) {
        perror("sigsuspend");
        exit(1);
      }
      printf("wake up parent\n");

      if(sigprocmask(SIG_SETMASK, &origMask, NULL) == -1) {

        perror("sigprocmask");
        exit(1);
      }
  }
  exit(EXIT_SUCCESS);
}

void handler(int no) {}
