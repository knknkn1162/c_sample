#include <sys/signalfd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h> // MAX
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 100

long doCalc(long a);

int main(int argc, char *argv[]) {
  char buf[BUF_SIZE];
  long num;
  int sfd, maxfd;
  fd_set fds;
  sigset_t mask;

  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  // when fd = -1, create new file descriptor
  if((sfd = signalfd(-1, &mask, 0)) == -1) {
    perror("signalfd");
    exit(1);
  }
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  FD_SET(sfd, &fds);
  

  maxfd = MAX(sfd, STDIN_FILENO) + 1;
  while(1) {
    if(select(maxfd, &fds, NULL, NULL, NULL) == -1) {
      perror("select");
    }

    // when signal catches
    if(FD_ISSET(sfd, &fds)) {
      struct signalfd_siginfo fdsi;
      if(read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo)) {
        perror("read signal");
        exit(1);
      }
      if(fdsi.ssi_signo == SIGINT) {
        printf("signal catch\n");
        break;
      }
      continue;
    }
    // TODO: always catch this code
    if(FD_ISSET(STDIN_FILENO, &fds)) {
      if(fgets(buf, BUF_SIZE, stdin) == NULL) {
        perror("fgets");
        exit(1);
      }

      if(strlen(buf) > 20) {
        fprintf(stderr, "> [error] too big!\n");
        continue;
      }
      if(buf[0] == '\n') {
        continue;
      }
      
      num = atol(buf);
      printf("> %ld -> %ld\n", num, doCalc(num));
    }
  }

  return 0;
}

long doCalc(long num) {
  return num*2;
}
