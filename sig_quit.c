#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void signal_handler(int signo);

int main(void) {
  sigset_t newmask, oldmask, pendmask;

  sigemptyset(&newmask);
  sigaddset(&newmask, SIGQUIT);

  if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
    perror("SIG_BLOCK error");
  }

  sleep(5);

  if(sigpending(&pendmask) < 0) {
    perror("sigpending error");
  }

  if (sigismember(&pendmask, SIGQUIT))
    printf("\nSIGQUIT pending\n");

  if(sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
    perror("SIG_SETMASK error");
  }
  printf("SIG_QUIT unblocked\n");

  sleep(5);
  return 0;
}

void signal_handler(int signo) {
  fprintf(stdout, "catcht SIGQUIT\n");
}
