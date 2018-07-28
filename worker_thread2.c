#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define NUM_THREADS 5
#define MAX_QUEUE_SIZE 100

typedef void* (*p_routine_t)(void*);

typedef struct tpool_work {
  p_routine_t routine;
  void* arg;
  int index;
  struct tpool_work* next;
} tpool_work_t;

typedef struct tpool {
  pthread_t threads[NUM_THREADS];
  int cur_queue_size;
  tpool_work_t* queue_head;
  tpool_work_t* queue_tail;
  pthread_mutex_t queue_lock;
  pthread_cond_t queue_not_empty;
  pthread_cond_t queue_not_full;
} tpool_t;

int tpool_init(tpool_t**);
int tpool_destroy(tpool_t*);
void* tpool_thread(void* arg);
void* add_work(tpool_t* p_tpool, p_routine_t routine, int index, void* arg);

void* sample_work(void* arg) {
  int m = *(int*)arg;
  sleep(5);
  fprintf(stdout, "%d\n", m);
  return NULL;
}

int main(void) {
  tpool_t* tpool;
  int m;
  int index = 0;
  char buf[256];
  if(tpool_init(&tpool) < 0) {
    perror("tpool_init error");
    exit(1);
  }

  while(1) {
    write(STDOUT_FILENO, "put number -> ", 15);
    fgets( buf, sizeof( buf ), stdin );

    if( strlen( buf ) <= 1 ) {
        continue;
    }
    if( memcmp( buf, "quit", 4 ) == 0 ) {
        break;
    }
    m = atoi(buf);
    fprintf(stdout, "number is:%d\n", m);
    add_work(tpool, sample_work, index++, (void*)&m);
  }

  tpool_destroy(tpool);
  return 0;
}

int tpool_init(tpool_t** pp_tpool) {
  int i;
  tpool_t* p_tpool = (tpool_t*)malloc(sizeof(tpool_t));

  p_tpool->cur_queue_size = 0;
  p_tpool->queue_head = NULL;
  p_tpool->queue_tail = NULL;
  pthread_mutex_init(&p_tpool->queue_lock, NULL);
  pthread_cond_init(&p_tpool->queue_not_empty, NULL);
  pthread_cond_init(&p_tpool->queue_not_full, NULL);

  for(i = 0; i < NUM_THREADS; i++) {
    pthread_create(&(p_tpool->threads[i]), NULL, tpool_thread, (void*)p_tpool);
  }

  *pp_tpool = p_tpool;
  return 0;
}

int tpool_destroy(tpool_t* p_tpool) {
  printf("graceful shutdown...\n");
  return 0;
}

void* tpool_thread(void* arg) {
  tpool_t* p_tpool = (tpool_t*)arg;
  tpool_work_t* my_workp;
  while(1) {
    pthread_mutex_lock(&p_tpool->queue_lock);
    while(p_tpool->cur_queue_size == 0) {
      fprintf(stdout, "ready tpool_thread\n");
      pthread_cond_wait(&p_tpool->queue_not_empty, &p_tpool->queue_lock);
      fprintf(stdout, "start tpool_thread %d\n", p_tpool->cur_queue_size);
    }

    my_workp = p_tpool->queue_head;
    p_tpool->cur_queue_size--;
    if(p_tpool->cur_queue_size == 0) {
      p_tpool->queue_head = p_tpool->queue_tail = NULL;
    } else {
      p_tpool->queue_head = my_workp->next;
    }

    if(p_tpool->cur_queue_size == MAX_QUEUE_SIZE - 1) {
      pthread_cond_signal(&p_tpool->queue_not_full);
    }
    pthread_mutex_unlock(&p_tpool->queue_lock);


    (*my_workp->routine)(my_workp->arg);
    free(my_workp);
  }
  return NULL;
}

void* add_work(tpool_t* p_tpool, p_routine_t routine, int index, void* arg) {
  tpool_work_t* workp;
  pthread_mutex_lock(&p_tpool->queue_lock);
  if(p_tpool->cur_queue_size == MAX_QUEUE_SIZE) {
    pthread_cond_wait(&p_tpool->queue_not_full, &p_tpool->queue_lock);
  }

  workp = (tpool_work_t*)malloc(sizeof(tpool_work_t));
  workp->routine = routine;
  workp->index = index;
  workp->arg = arg;
  workp->next = NULL;

  if(p_tpool->cur_queue_size == 0) {
    p_tpool->queue_tail = p_tpool->queue_head = workp;
    printf("emit add_work signal\n");
    pthread_cond_signal(&p_tpool->queue_not_empty);
  } else {
    p_tpool->queue_tail = workp;
    p_tpool->queue_tail->next = p_tpool->queue_head;
  }

  p_tpool->cur_queue_size++;
  pthread_mutex_unlock(&p_tpool->queue_lock);
  return NULL;
}
