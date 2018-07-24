#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void signal_handler(int sig);
void abort();

int main() {
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGABRT, &sa, 0);

  printf("in mail before abort\n");
  abort();
  printf("In main after abort\n");
}

void signal_handler(int sig) {
  char* msg = "In signal handler!!\n";
  write(1, msg, strlen(msg));
}

void abort(void) {
  sigset_t mask;
  struct sigaction sa;

  sigaction(SIGABRT, 0, &sa);
  if(sa.sa_handler == SIG_IGN) {
    sa.sa_handler = SIG_DFL;
    sigaction(SIGABRT, &sa, 0);
  }

  kill(getpid(), SIGABRT);

  sa.sa_handler = SIG_DFL;
  sigaction(SIGABRT, &sa, 0);
  sigfillset(&mask);
  sigdelset(&mask, SIGABRT);
  sigprocmask(SIG_SETMASK, &mask, 0);

  kill(getpid(), SIGABRT);
  exit(1);
}
