#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_MAX 256

void* temp_func(void *arg);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready_cond = PTHREAD_COND_INITIALIZER;

int start_flg = 0;

int main(int argc, char** argv) {
  pthread_t pt[THREAD_MAX];
  int all_thr_count = atoi(argv[1]);
  int i;

  pthread_mutex_lock(&ready_mutex);
  for(i = 0; i < all_thr_count; i++) {
    pthread_create(&pt[i], NULL, temp_func, &i);
    pthread_cond_wait(&ready_cond, &ready_mutex);
  }
  pthread_mutex_unlock(&ready_mutex);

  start_flg = 1;
  pthread_cond_broadcast(&cond);
  for(i = 0; i < all_thr_count; i++) {
    pthread_join(pt[i], 0);
  }

  return 0;
}

void* temp_func(void* arg) {
  pthread_mutex_lock(&ready_mutex);
  int index = *(int*)arg;
  pthread_cond_signal(&ready_cond);
  pthread_mutex_unlock(&ready_mutex);

  write(STDOUT_FILENO, "start thread index\n", 20);

  pthread_mutex_lock(&mutex);
  while(start_flg == 0) {
    pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  fprintf(stdout, "%d output\n", index);
  return 0;
}
