#include <sys/signalfd.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  sigset_t mask;
  struct signalfd_siginfo fdsi;
  int sfd;
  int num;

  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);

  if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }
  // int signalfd(int fd, const sigset_t *mask, int flags);
  // when fd = -1, create new file descriptor
  sfd = signalfd(-1, &mask, 0);
  if(sfd == -1) {
    perror("signalfd");
    exit(1);
  }

  while(1) {
    if(read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo)) {
      perror("read");
      exit(1);
    }
    switch(fdsi.ssi_signo) {
      case SIGINT:
        printf("[SIGINT] signal: %d sent by pid, %d\n", fdsi.ssi_signo, fdsi.ssi_pid);
        break;
      case SIGTERM:
        printf("[SIGTERM] signal: %d sent by pid, %d\n", fdsi.ssi_signo, fdsi.ssi_pid);
        break;
      default:
        printf("other\n");
        exit(EXIT_SUCCESS);
    }
  }

  return 0;
}
