#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 256

void* reactor(void*);
void* worker(void*);


static pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;
static int count = 0;


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

  if(pthread_join(pt, &res) != 0) {
    perror("pthread_join");
    exit(1);
  }
  if(res == PTHREAD_CANCELED) {
    printf("[main thread]: reactor was cancelled\n");
  }

  return 0;
}



void* reactor(void* arg) {
  char *buf;
  // do sth
  while(1) {
    pthread_t pt_worker;
    buf = malloc(BUF_SIZE);
    fgets(buf, sizeof(buf), stdin);

    // cancellation succeed
    pthread_create(&pt_worker, NULL, &worker, (void*)buf);
    pthread_detach(pt_worker);
  }

  pthread_exit(NULL);
}

// TODO: don't react cancellation, while main thread abort immediately.
void* worker(void* arg) {
  char buf[BUF_SIZE];
  int cnt;
  strncpy(buf, (char*)arg, BUF_SIZE);
  // allocate arg by reactor()
  free(arg);

  // assume to take some time..
  sleep(2);
  pthread_mutex_lock(&threadMutex);
  cnt = count++;
  pthread_mutex_unlock(&threadMutex);
  printf("[%d] > %s", cnt, buf);
  pthread_exit(NULL);
}
