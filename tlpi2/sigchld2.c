#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static volatile int numLiveChildren = 0;
#define CHILD_NUM 40

static void handler(int sig) {
  pid_t childPid;
  write(STDOUT_FILENO, "o", 1);
  while((childPid = waitpid(-1, NULL, WNOHANG)) > 0) {
    write(STDOUT_FILENO, "*", 1);
    numLiveChildren--;
  }
  if(childPid == -1 && errno != ECHILD) {
    perror("waitpid");
  }
}

int main(int argc, char *argv[]) {
  int sigCnt = 0;
  int i;
  struct sigaction sa;
  sigset_t blockMask, emptyMask;
  numLiveChildren = CHILD_NUM;
  setbuf(stdout, NULL);


  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  sigemptyset(&blockMask);
  sigaddset(&blockMask, SIGCHLD);
  if(sigprocmask(SIG_SETMASK, &blockMask, NULL) == -1) {
    perror("sigprocmasK");
    exit(1);
  }

  for(i = 0; i < CHILD_NUM; i++) {
    switch(fork()) {
      case -1:
        perror("fork");
        exit(1);
      case 0:
        sleep(atoi(argv[1]));
        printf("child: %d\n", getpid());
        _exit(EXIT_SUCCESS);
      default:
        break;
    }
  }

  sigemptyset(&emptyMask);
  while(numLiveChildren > 0) {
    if(sigsuspend(&emptyMask) == -1 && errno != EINTR) {
      perror("sigsuspend");
      exit(1);
    }
    sigCnt++;
  }

  printf("sigcnt: %d\n", sigCnt);
  return 0;
}
