#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_MAX 8

typedef struct control_tag {
  int counter;
  int busy;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} control_t;

void* thread_func(void* arg);
void cleanup_handler(void* arg);

int main(void) {
  control_t control;
  pthread_t id[THREAD_MAX];
  void* result;
  int i;

  control.counter = 0;
  control.busy = 1;
  pthread_mutex_init(&control.mutex, NULL);
  pthread_cond_init(&control.cond, 0);

  for(i = 0; i < THREAD_MAX; i++) {
    pthread_create(&id[i], NULL, thread_func, (void*)&control);
  }

  sleep(2);

  for(i = 0; i < THREAD_MAX; i++) {
    pthread_cancel(id[i]);
    pthread_join(id[i], &result);
    if(result == PTHREAD_CANCELED) {
      fprintf(stdout, "thread %d cancelled\n", i);
    } else {
      fprintf(stdout, "thread %d was not cancelled\n", i);
    }
  }

  return 0;
}

void* thread_func(void* arg) {
  control_t* control = (control_t*)arg;
  pthread_cleanup_push(cleanup_handler, arg);
  pthread_mutex_lock(&control->mutex);

  // do sth
  control->counter++;

  while(control->busy) {
    fprintf(stdout, "thread waiting...\n");
    pthread_cond_wait(&control->cond, &control->mutex);
  }

  pthread_mutex_unlock(&control->mutex);
  pthread_cleanup_pop(1);

  return 0;
}


void cleanup_handler(void* arg) {
  control_t* control = (control_t*)arg;
  control->counter--;
  fprintf(stdout, "cleanup_handler: counter[%d]\n", control->counter);
  pthread_mutex_unlock(&control->mutex);
}
