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
static pthread_cond_t condBuf = PTHREAD_COND_INITIALIZER;
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
  char buf[BUF_SIZE];
  // do sth
  while(1) {
    pthread_t pt_worker;
    fgets(buf, sizeof(buf), stdin);

    // TODO: If the cancellation occurs in the lock, When is the mutex unlocked until exited the program?
    pthread_mutex_lock(&threadMutex);
    pthread_create(&pt_worker, NULL, &worker, (void*)buf);
    pthread_cond_wait(&condBuf, &threadMutex);
    pthread_mutex_unlock(&threadMutex);
    pthread_detach(pt_worker);
  }

  pthread_exit(NULL);
}

void* worker(void* arg) {
  char buf[BUF_SIZE];

  pthread_mutex_lock(&threadMutex);
  strncpy(buf, (char*)arg, BUF_SIZE);
  pthread_mutex_unlock(&threadMutex);
  pthread_cond_signal(&condBuf);

  // assume to take some time..
  sleep(2);
  pthread_mutex_lock(&threadMutex);
  printf("[%d] > %s", count++, buf);
  pthread_mutex_unlock(&threadMutex);
  pthread_exit(NULL);
}
