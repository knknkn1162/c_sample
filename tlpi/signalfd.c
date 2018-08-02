#include <sys/signalfd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  sigset_t mask;
  int sfd, j;
  struct signalfd_siginfo fdsi;
  ssize_t s;

  printf("PID = %d\n", getpid());

  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);

  if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
    perror("sigprocmask");
  }


  sfd = signalfd(-1, &mask, 0);
  if(sfd == -1) {
    perror("signalfd");
    exit(1);
  }

  while(1) {
    s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
    if(s != sizeof(struct signalfd_siginfo)) {
      perror("read");
      exit(1);
    }

    printf("got signal %d\n", fdsi.ssi_signo);

    if(fdsi.ssi_code == SI_QUEUE) {
      printf("ssi_pid: %d\n", fdsi.ssi_pid);
      printf("ssi_int = %d\n", fdsi.ssi_int);
    }
  }

  exit(EXIT_SUCCESS);
}
