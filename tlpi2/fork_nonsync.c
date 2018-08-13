#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#define SYNC_SIG SIGUSR1

static void handler(int sig) {}
void doSomething() { sleep(40); }


int main(int argc, char *argv[]) {
  pid_t pid;
  struct sigaction sa;
  sigset_t emptyMask;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handler;
  if(sigaction(SYNC_SIG, &sa, NULL) == -1) {
    perror("sigaction");
  }

  switch(pid = fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      // child process
      doSomething();
      // or you may use sigqueue
      if(kill(getppid(), SYNC_SIG) == -1) {
        perror("kill");
        exit(1);
      }

      // For the reason that we use _exit rather than exit, see TLPI 25.4
      _exit(EXIT_SUCCESS);
    default:
      sigemptyset(&emptyMask);
      //parent process
      // WRONG: Terminate if the parent process receive SYNC_SIG signal before doSomething in the child process. Try `kill -SIGUSR1 $(PARENT_PROCESS)`.
      if(sigsuspend(&emptyMask) == -1 && errno != EINTR) {
        perror("sigsuspend");
        exit(1);
      }
      printf("got signal\n");
      exit(EXIT_SUCCESS);
  }
}
