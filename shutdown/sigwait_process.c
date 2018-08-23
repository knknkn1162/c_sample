#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 100

long doCalc(long a);

int main(int argc, char *argv[]) {
  char buf[BUF_SIZE];
  long num;
  sigset_t ss;
  pid_t pid;
  sigemptyset(&ss);
  sigaddset(&ss, SIGINT);
  if(sigprocmask(SIG_SETMASK, &ss, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  if((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    // child
    while(1) {
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
    _exit(EXIT_SUCCESS);
  } else {
    // parent
    int sig;
    int flag = 1;
    while(flag) {
      printf("[parent] sigwait..\n");
      if(sigwait(&ss, &sig) != 0) {
        continue;
      }
      switch(sig) {
        case SIGINT:
          printf("[parent] catch sigint\n");
          flag = 0;
          // TODO: how to shutdown the child process?
          break;
        default:
          break;
      }
    }
  }

  wait(NULL);
  return 0;
}

long doCalc(long num) {
  return num*2;
}
