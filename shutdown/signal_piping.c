#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h> // MAX
#include <sys/signalfd.h>
#include <errno.h>


static int sig_pipe[2];
static void signal_handler(int signo, siginfo_t *info, void *w) {
  write(sig_pipe[1], info, sizeof(siginfo_t));
}
long doCalc(long num);
void doShutdown(void);

#define BUF_SIZE 100


int main(void) {
  struct sigaction sa;
  fd_set fds, tmp_fds;
  int maxfd;
  int flags;
  char buf[BUF_SIZE];

  pipe(sig_pipe);
  flags = fcntl(sig_pipe[0], F_GETFL, 0);
  if(flags == -1) {
    perror("fcntl F_GETFL");
    exit(1);
  }

  if(fcntl(sig_pipe[0], F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL");
    exit(1);
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = signal_handler;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGINT, &sa, NULL);

  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  FD_SET(sig_pipe[0], &fds);
  maxfd = MAX(sig_pipe[0], STDIN_FILENO) + 1;

  while (1) {
    long num;
    siginfo_t info;
    memcpy(&tmp_fds, &fds, sizeof(fd_set));
    if(select(maxfd, &tmp_fds, NULL, NULL, NULL) == -1) {
      if(errno == EINTR) {
        continue;
      } else {
        perror("select");
        exit(1);
      }
    }

    if(FD_ISSET(sig_pipe[0], &tmp_fds)) {
      if (read(sig_pipe[0], &info, sizeof(siginfo_t)) > 0) {
        if(info.si_signo == SIGINT) {
          fprintf(stderr, "signal catch\n");
          break;
        }
      }
      continue;
    }

    if(FD_ISSET(STDIN_FILENO, &tmp_fds)) {
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
  doShutdown();
  return 0;
}

long doCalc(long num) {
  return num*2;
}

void doShutdown(void) {
  sigset_t mask, prevMask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK, &mask, &prevMask);
  printf("graceful shutdown..\n");
  // go shutdown..
  sleep(2);
  sigprocmask(SIG_SETMASK, &prevMask, 0);
}
