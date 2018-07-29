#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

static long long counter;
void* thread_func(void* arg);

int main(void) {
  pthread_t thread_id;
  void* result = NULL;

  pthread_create(&thread_id, NULL, thread_func, NULL);

  sleep(2);

  pthread_cancel(thread_id);
  fprintf(stdout, "cancel");
  pthread_join(thread_id, &result);

  if(result == PTHREAD_CANCELED) {
    fprintf(stdout, "\ncancelled at iteration %lld\n", counter);
  } else {
    fprintf(stdout, "Thread was not cancelled\n");
  }
  return 0;
}

void* thread_func(void* arg) {
  for(counter = 0; ; counter++) {
    if(counter%1000 == 0) {
      pthread_testcancel();
    }
  }

  return 0;
}
