#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static volatile sig_atomic_t gotSigio = 0;
static void sigioHandler(int sig);
#define BUF_SIZE 10

int main(int argc, char *argv[]) {

  struct sigaction sa;
  char buf[BUF_SIZE];
  int flags;
  int i;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = sigioHandler;
  if(sigaction(SIGIO, &sa, NULL) == -1) {
    perror("sigaction");
  }

  // set owner process that receives signal
  if(fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1) {
    perror("fcntl setown");
    exit(1);
  }

  flags = fcntl(STDIN_FILENO, F_GETFL);
  // Set NONBLOCK because the read function may block when no more I/O is possible
  if(fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1) {
    perror("fcntl");
    exit(1);
  }

  while(1) {
    if(gotSigio) {
      while(read(STDIN_FILENO, buf, BUF_SIZE - 1) > 0) {
        for(i = 0; i < BUF_SIZE - 1; i++) {
          if(buf[i] == '\0' || buf[i] == '\n') { break; }
        }
        if(i != BUF_SIZE-1) {
          buf[i] = '\n';
        }
        write(STDOUT_FILENO, buf, i+1);
      }
      gotSigio = 0;
    }
  }
  return 0;
}

static void sigioHandler(int sig) {
  gotSigio = 1;
}
