#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int glob = 0;
static sem_t sem;

static void *threadfunc(void *arg);

int main(int argc, char *argv[]) {
  pthread_t t1, t2;
  int loops = 10000000;

  if(sem_init(&sem, 0, 1) == -1) {
    perror("sem_init");
    exit(1);
  }

  if(pthread_create(&t1, NULL, threadfunc, &loops) != 0) {
    perror("pthread_create 1");
    exit(1);
  }

  if(pthread_create(&t2, NULL, threadfunc, &loops) != 0) {
    perror("pthread_create 1");
    exit(1);
  }

  if(pthread_join(t1, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }

  if(pthread_join(t2, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }

  printf("glob = %d\n", glob);
  exit(EXIT_SUCCESS);
}

static void *threadfunc(void *arg) {
  int loops = *((int*)arg);
  int loc, j;

  for(j = 0; j < loops; j++) {
    if(sem_wait(&sem) == -1) {
      perror("sem_wait");
    }

    loc = glob;
    loc++;
    glob = loc;

    if(sem_post(&sem) == -1) {
      perror("sem_post");
    }
  }

  return NULL;
}
