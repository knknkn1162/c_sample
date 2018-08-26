#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 64


sig_atomic_t gotSig = 0;
void handler(int sig) { gotSig = 1; }


int main(int argc, char *argv[]) {
  struct sigaction sa;
  int nfds, ready;
  char buf[BUF_SIZE];
  fd_set readfds;
  fd_set readfds_tmp;
  sigset_t emptyset, blockset;
  sigemptyset(&emptyset);

  sigemptyset(&blockset);
  sigaddset(&blockset, SIGINT);

  if(sigprocmask(SIG_BLOCK, &blockset, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }


  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);
  nfds = STDIN_FILENO + 1;
  while(1) {
    memcpy(&readfds_tmp, &readfds, sizeof(fd_set));
    if((ready = pselect(nfds, &readfds_tmp, NULL, NULL, NULL, &emptyset)) == -1) {
      if(errno == EINTR) {
        if(gotSig) {
          printf("gotSignal");
          exit(1);
        }
      } else {
        continue;
      }
    } else if(ready > 0) {
      int numRead = read(STDIN_FILENO, buf, BUF_SIZE);
      write(STDOUT_FILENO, buf, numRead);
    }
  }

  return 0;
}

