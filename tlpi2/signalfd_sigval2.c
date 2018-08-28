#include <sys/signalfd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


int main(int argc, char *argv[]) {
  sigset_t mask;
  int sfd;
  struct signalfd_siginfo fdsi;

  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }
  sfd = signalfd(-1, &mask, 0);
  if(sfd == -1) {
    perror("signalfd");
  }

  while(1) {
    int num = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
    if(num != sizeof(struct signalfd_siginfo)) {
      perror("read");
    }
    // ssi_signo, ssi_code
    if(fdsi.ssi_signo == SIGINT) {
      fprintf(stderr, "sigint\n");
    }
    printf("by %d\n", fdsi.ssi_pid);
    switch(fdsi.ssi_code) {
      // kill
      case SI_USER:
        fprintf(stdout, "si_user\n");
        break;
      // sigqueue
      case SI_QUEUE:
        fprintf(stdout, "si_queue\n");
        break;
      // Ctrl-C process 0
      case SI_KERNEL:
        fprintf(stdout, "kernel\n");
        break;
    }
  }


  return 0;
}
