#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static int glob = 0;
static sem_t sem;

static void *threadFunc(void *arg);

int main(int argc, char *argv[]) {

  pthread_t t1, t2;

  // int sem_init(sem_t *sem, int pshared, unsigned int value);
  if(sem_init(&sem, 0, 1) == -1) {
    perror("sem_init");
    exit(1);
  }

  if(pthread_create(&t1, NULL, threadFunc, NULL) != 0) {
    perror("pthread_create");
    exit(1);
  }

  if(pthread_create(&t2, NULL, threadFunc, NULL) != 0) {
    perror("pthread_create");
    exit(1);
  }

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("glob = %d\n", glob);

  return 0;
}

static void *threadFunc(void *arg) {
  int j;
  int loc;
  for(j = 0; j < 1000; j++) {
    // decrements the value of the semaphore
    // If the value of the semaphore is 0, blocking, otherwise, immediately returns
    if(sem_wait(&sem) == -1) {
      perror("sem_wait");
    }
    loc = glob;
    loc++;
    glob = loc;
    // increments the value of the semaphore
    if(sem_post(&sem) == -1) {
      perror("sem_post");
      exit(1);
    }
  }
  return NULL;
}
