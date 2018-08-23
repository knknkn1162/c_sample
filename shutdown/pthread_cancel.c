#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 128
#define DIGIT_MAX 20

void* reactor(void*);
long doCalc(long a);

int main(int argc, char *argv[]) {
  pthread_t pt;
  sigset_t ss;
  int sig;
  void* res;
  int flag = 1;

  sigemptyset(&ss);
  sigaddset(&ss, SIGINT);
  sigprocmask(SIG_BLOCK, &ss, NULL);

  pthread_create(&pt, NULL, &reactor, NULL);

  sigemptyset(&ss);
  sigaddset(&ss, SIGINT);
  while(flag) {
    printf("[main thread] sigwait ready..\n");
    if(sigwait(&ss, &sig)) {
      continue;
    }
    switch(sig) {
      case SIGINT:
        printf("[main thread] catch sigint\n");
        flag = 0;
        break;
      default:
        break;
    }
  }
  printf("[main thread] exit\n");
  if(pthread_cancel(pt) != 0) {
    perror("pthread_cancel");
    exit(1);
  }

  // termination
  if(pthread_join(pt, &res) != 0) {
    perror("pthread_join");
    exit(1);
  }
  if(res == PTHREAD_CANCELED) {
    printf("[main thread]: reactor was cancelled\n");
  }

  return 0;
}

long doCalc(long num) {
  return num*2;
}

void* reactor(void* arg) {
  char buf[BUF_SIZE];
  long num;
  while(1) {
    if(fgets(buf, BUF_SIZE, stdin) == NULL) {
      perror("fgets");
      exit(1);
    }

    if(strlen(buf) > DIGIT_MAX) {
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
