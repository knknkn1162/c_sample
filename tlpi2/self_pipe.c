#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/param.h>
#include <fcntl.h>

static int pfd[2];
static void handler(int sig) {
  int savedErrno;

  savedErrno = errno;
  if(write(pfd[1], "x", 1) == -1 && errno != EAGAIN) {
    perror("write");
    exit(1);
  }
  errno = savedErrno;
}


int main(int argc, char *argv[]) {

  int nfds;
  int flags;
  int i;
  int ready;
  struct sigaction sa;
  char ch;
  fd_set readfds;

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  FD_SET(pfd[0], &readfds);
  nfds = pfd[0] + 1;

  for(i = 0; i < 2; i++) {
    flags = fcntl(pfd[i], F_GETFL);
    if(flags == -1) {
      perror("fcntl");
      exit(1);
    }

    flags |= O_NONBLOCK;
    if(fcntl(pfd[i], F_SETFL, flags) == -1) {
      perror("fcntl F_SETFL");
      exit(1);
    }
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    if((ready = select(nfds, &readfds, NULL, NULL, NULL)) == -1) {
      if(errno == EINTR) {
        continue;
      }
      perror("select");
      break;
    }

    if(FD_ISSET(pfd[0], &readfds)) {
      printf("Signal was caught\n");

      while(1) {
        if(read(pfd[0], &ch, 1) == -1) {
          if(errno == EAGAIN) {
            printf("eagain\n");
            break;
          }
          perror("read");
          exit(1);
        } else {
          printf("> %c \n", ch);
          break;
        }
      }
    }
  }

  return 0;
}
