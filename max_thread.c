#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void* temp_func(void* arg);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main() {
  int i = 0;
  int rtn = 0;
  pthread_t id;

  pthread_mutex_lock(&mutex);
  for(i = 0;; i++) {
    rtn = pthread_create(&id, 0, temp_func, (void*)&i);
    if(rtn) {
      printf("rtn: %d, i: %d", rtn, i);
      break;
    }
    // wait until pthread_cond_signal
    pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);
  fprintf(stdout, "max thread count[%d]\n", i - 1);
  return 0;
}

void* temp_func(void* arg) {
  pthread_mutex_lock(&mutex);
  int cnt = *(int*)arg;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  if(cnt % 1000 == 0 && cnt != 0) {
    fprintf(stdout, "temp_func: [%4d]\n", cnt);
  }

  while(1) {
    sleep(3600);
  }
  pthread_exit(0);
}
