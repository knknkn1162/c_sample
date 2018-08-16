
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

static pthread_cond_t threadDied = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;


static int totThreads = 0;
static int numLive = 0;

static int numUnjoined = 0;

enum tstate {
  TS_ALIVE,
  TS_TERMINATED,
  TS_JOINED
};

struct threadInfo {
  pthread_t tid;
  enum tstate state;
  int sleepTime;
};

static struct threadInfo* threads;

static void* threadFunc(void* arg);
#define THREAD_MAX  256

int main(int argc, char *argv[]) {

  int s, idx;
  int index[THREAD_MAX];
  
  threads = calloc(argc-1, sizeof(struct threadInfo));
  if(threads == NULL) {
    perror("calloc");
    exit(1);
  }

  for(idx = 0; idx < argc - 1; idx++) {
    index[idx] = idx;
    threads[idx].sleepTime = atoi(argv[idx+1]);
    threads[idx].state = TS_ALIVE;
    // int pthread_create(pthread_t * thread, pthread_attr_t * attr, void * (*start_routine)(void *), void * arg);
    s = pthread_create(&threads[idx].tid, NULL, threadFunc, (void*)&index[idx]);
    if(s != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  totThreads = argc - 1;
  numLive = totThreads;

  printf("start\n");
  while(numLive > 0) {
    s = pthread_mutex_lock(&threadMutex);
    if(s != 0) {
      perror("pthread_mutex_lock");
    }

    while(numUnjoined == 0) {
      // These functions atomically release mutex and cause the calling thread to block on the condition variable cond
      s = pthread_cond_wait(&threadDied, &threadMutex);
      if(s != 0) {
        perror("pthread_cond_wait");
        exit(1);
      }
    }

    for(idx = 0; idx < totThreads; idx++) {
      if(threads[idx].state == TS_TERMINATED) {
        s = pthread_join(threads[idx].tid, NULL);
        if(s != 0) {
          perror("pthread_join");
        }
        threads[idx].state = TS_JOINED;
        numLive--;
        numUnjoined--;

        printf("Reaped thread %d(numLive=%d)\n", idx, numLive);
      }
    }

    s = pthread_mutex_unlock(&threadMutex);
    if(s != 0) {
      perror("pthread_mutex_unlock");
      exit(1);
    }
  }
  exit(EXIT_SUCCESS);

}

static void* threadFunc(void* arg) {
  int s;
  int idx = *((int*)arg);
  
  sleep(threads[idx].sleepTime);
  s = pthread_mutex_lock(&threadMutex);
  numUnjoined++;
  threads[idx].state = TS_TERMINATED;
  s = pthread_mutex_unlock(&threadMutex);

  s = pthread_cond_signal(&threadDied);
  return NULL;
}
