#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t strerrorkey;

#define MAX_ERROR_LEN 256

static void destructor(void* buf) { free(buf); }
static void createKey(void) {
  int s;
  s = pthread_key_create(&strerrorkey, destructor);
  if(s != 0) {
    perror("pthread_create");
    exit(1);
  }
}

char* strerror(int err) {
  int s;
  char *buf;

  s = pthread_once(&once, createKey);
  if(s != 0) {
    perror("pthread_once");
    exit(1);
  }

  buf = pthread_getspecific(strerrorkey);
  if(buf == NULL) {
    buf = malloc(MAX_ERROR_LEN);
    if(buf == NULL) {
      perror("malloc");
      exit(1);
    }
    s = pthread_setspecific(strerrorkey, buf);
    if(s != 0) {
      perror("pthread_setspecific");
    }
  }

  if(err < 0 || err >= _sys_nerr || _sys_errlist[err] == NULL) {
    printf("unknown error");
  } else {
    strncpy(buf, _sys_errlist[err], MAX_ERROR_LEN - 1);
    buf[MAX_ERROR_LEN - 1] = '\0';
  }

  return buf;
}

int main(void) {return 0;}
