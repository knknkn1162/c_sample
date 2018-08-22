#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int glob = 0;


static void cleanupHandler(void *arg);
static void *threadFunc(void *arg);

int main(int argc, char *argv[]) {
  pthread_t pt;
  void *res;

  if(pthread_create(&pt, NULL, threadFunc, NULL) != 0) {
    perror("pthread_create");
    exit(1);
  }


  sleep(2);
  printf("about to cancel thread\n");
  if(pthread_cancel(pt) != 0) {
    perror("pthread_cancel");
    exit(1);
  }

  if(pthread_join(pt, &res) != 0) {
    perror("pthread_join");
    exit(1);
  }

  if(res == PTHREAD_CANCELED) {
    printf("thread was canceled\n");
  }

  exit(EXIT_SUCCESS);
}

static void* threadFunc(void *arg) {
  void *buf = NULL;

  buf = malloc(0x10000);
  printf("thread: allocated memory at %p\n", buf);

  if(pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    exit(1);
  }
  pthread_cleanup_push(cleanupHandler, (void*)buf);

  while(glob == 0) {
    if(pthread_cond_wait(&cond, &mutex) != 0) {
      perror("pthread_cond_wait");
      exit(1);
    }
  }

  printf("thread: cond wait loop completed\n");
  // When  a thread calls pthread_cleanup_pop() with a nonzero execute argument, the top-most clean-up handler is popped and executed.
  pthread_cleanup_pop(1);

  pthread_exit(NULL);

}

static void cleanupHandler(void *arg) {
  printf("cleanup: freeing block at %p\n", arg);
  free(arg);

  printf("cleanup: unlocking mutex\n");
  if(pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    exit(1);
  }
}
