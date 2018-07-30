#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define NUM_THREADS 5
#define MAX_QUEUE_SIZE 100
#define WORKER_TIME 20

typedef void* (*p_routine_t)(void*);

typedef struct tpool_work {
  p_routine_t routine;
  void* arg;
  struct tpool_work* next;
} tpool_work_t;

typedef struct tpool {
  pthread_t workers[NUM_THREADS];
  int cur_queue_size;
  tpool_work_t* queue_head;
  tpool_work_t* queue_tail;
  pthread_mutex_t queue_lock;
  pthread_cond_t queue_not_empty;
  pthread_cond_t queue_not_full;
  int queue_shutdown;
} tpool_t;

int tpool_init(tpool_t**);
int tpool_destroy(tpool_t*);
void* tpool_thread(void* arg);
void* add_work(tpool_t* p_tpool, p_routine_t routine, void* arg);

void* sample_work(void* arg) {
  int m = *(int*)arg;
  sleep(WORKER_TIME);
  //fprintf(stdout, "answer = %d\n", m);
  printf("answer = %d\n", m);
  return NULL;
}

int main(void) {
  tpool_t* tpool;
  int* m;
  char buf[256];

  if(tpool_init(&tpool) < 0) {
    perror("tpool_init error");
    exit(1);
  }

  while(1) {
    // generate input resources
    m = malloc(sizeof(int));
    write(STDOUT_FILENO, "put number -> ", 15);
    fgets( buf, sizeof( buf ), stdin );

    if( strlen( buf ) <= 1 ) {
      continue;
    }
    if( memcmp( buf, "quit", 4 ) == 0 ) {
        break;
    }
    // TODO: graceful shutdown when Ctrl-C
    *m = strtol(buf, NULL, 10);
    if(*m == 0) {
      //fprintf(stdout, "cannot parse!!\n");
      printf("cannot parse!!\n");
      continue;
    }
    //fprintf(stdout, "input :%d\n", *m);
    printf("input :%d\n", *m);
    add_work(tpool, sample_work, (void*)m);
  }
  tpool_destroy(tpool);
  return 0;
}

int tpool_init(tpool_t** pp_tpool) {
  int i;
  // generate thread pool resources
  tpool_t* p_tpool = (tpool_t*)malloc(sizeof(tpool_t));

  p_tpool->cur_queue_size = 0;
  p_tpool->queue_head = NULL;
  p_tpool->queue_tail = NULL;
  p_tpool->queue_shutdown = 0;
  pthread_mutex_init(&p_tpool->queue_lock, NULL);
  pthread_cond_init(&p_tpool->queue_not_empty, NULL);
  pthread_cond_init(&p_tpool->queue_not_full, NULL);

  for(i = 0; i < NUM_THREADS; i++) {
    pthread_create(&(p_tpool->workers[i]), NULL, tpool_thread, (void*)p_tpool);
  }

  *pp_tpool = p_tpool;
  return 0;
}

int tpool_destroy(tpool_t* p_tpool) {
  int i;
  tpool_work_t* cur_nodep;

  if(p_tpool->queue_shutdown) {
    return 1;
  }

  //fprintf(stdout, "graceful shutdown...\n");
  printf("graceful shutdown...\n");

  // turn on shutdown flag
  p_tpool->queue_shutdown = 1;

  // wakeup WAIT status thread
  pthread_cond_broadcast(&p_tpool->queue_not_empty);
  pthread_cond_broadcast(&p_tpool->queue_not_full);

  // shutdown workers
  for(i = 0; i < NUM_THREADS; i++) {
    pthread_join(p_tpool->workers[i], NULL);
  }


  // destroy worker. You cannot use for expression.
  while(p_tpool->queue_head != NULL) {
    cur_nodep = p_tpool->queue_head;
    p_tpool->queue_head = p_tpool->queue_head->next;
    p_tpool->cur_queue_size--;
    // destroy input
    free(cur_nodep->arg);
    // destroy job
    free(cur_nodep);
  }
  //printf(stdout, "queue_size: %d\n", p_tpool->cur_queue_size);
  printf("queue_size: %d\n", p_tpool->cur_queue_size);

  // destroy thread pool resources
  free(p_tpool);
  //fprintf(stdout, "destroy thread pool resources!!\n");
  printf("destroy thread pool resources!!\n");
  return 0;
}

void* tpool_thread(void* arg) {
  // thread number
  int idx;
  tpool_t* p_tpool = (tpool_t*)arg;
  tpool_work_t* my_workp;

  for(idx = 0; idx < NUM_THREADS; idx++) {
    if(p_tpool->workers[idx] == pthread_self()) {
      break;
    }
  }
  while(1) {
    pthread_mutex_lock(&p_tpool->queue_lock);

    while(p_tpool->cur_queue_size == 0 && p_tpool->queue_shutdown == 0) {
      //fprintf(stdout, "ready tpool_thread %d\n", idx);
      printf("ready tpool_thread %d\n", idx);
      pthread_cond_wait(&p_tpool->queue_not_empty, &p_tpool->queue_lock);
    }

    if(p_tpool->queue_shutdown == 1) {
      pthread_mutex_unlock(&p_tpool->queue_lock);
      printf("end thread: %d\n", idx);
      pthread_exit(NULL);
    }

    //fprintf(stdout, "start tpool_thread %d: cur_queue_size: %d\n", idx, p_tpool->cur_queue_size);
    printf("start tpool_thread %d: cur_queue_size: %d\n", idx, p_tpool->cur_queue_size);

    // fetch job
    my_workp = p_tpool->queue_head;
    p_tpool->cur_queue_size--;

    if(p_tpool->cur_queue_size == 0) {
      p_tpool->queue_head = p_tpool->queue_tail = NULL;
    } else {
      //fprintf(stdout, "cur_queue_size > 0\n");
      printf("cur_queue_size > 0\n");
      p_tpool->queue_head = my_workp->next;
    }

    if(p_tpool->cur_queue_size == MAX_QUEUE_SIZE - 1) {
      pthread_cond_signal(&p_tpool->queue_not_full);
    }
    pthread_mutex_unlock(&p_tpool->queue_lock);


    //fprintf(stdout, "* start routine: %d\n", idx);
    printf("* start routine: %d\n", idx);
    (*my_workp->routine)(my_workp->arg);
    // destroy input resources
    free(my_workp->arg);
    free(my_workp);
    //fprintf(stdout, "* end routine: %d\n", idx);
    printf("* end routine: %d\n", idx);
  }
  return NULL;
}

void* add_work(tpool_t* p_tpool, p_routine_t routine, void* arg) {
  tpool_work_t* workp;
  tpool_work_t* tmp;

  pthread_mutex_lock(&p_tpool->queue_lock);
  if(p_tpool->cur_queue_size == MAX_QUEUE_SIZE) {
    pthread_cond_wait(&p_tpool->queue_not_full, &p_tpool->queue_lock);
  }

  if(p_tpool->queue_shutdown == 1) {
    //fprintf(stdout, "shutdown queue..");
    printf("shutdown queue..");
    pthread_mutex_unlock(&p_tpool->queue_lock);
    exit(1);
  }

  // create job
  workp = (tpool_work_t*)malloc(sizeof(tpool_work_t));
  workp->routine = routine;
  workp->arg = arg;
  workp->next = NULL;

  // push tail
  //fprintf(stdout, "queue_size: %d\n", p_tpool->cur_queue_size);
  printf("queue_size: %d\n", p_tpool->cur_queue_size);
  if(p_tpool->cur_queue_size == 0) {
    p_tpool->queue_tail = p_tpool->queue_head = workp;
    //fprintf(stdout, "queue_size: 0->emit queue_not_empty\n");
    printf("queue_size: 0->emit queue_not_empty\n");
    pthread_cond_signal(&p_tpool->queue_not_empty);
  } else {
    if (p_tpool->queue_head == p_tpool->queue_tail) {
      p_tpool->queue_tail = workp;
      p_tpool->queue_head->next = workp;
    } else
      p_tpool->queue_tail->next = workp;
      p_tpool->queue_tail = workp;
    }
    for(tmp = p_tpool->queue_head; tmp; tmp = tmp->next) {
      //fprintf(stdout, "%p arg: %d\n",tmp,  *((int*)tmp->arg));
      printf("%p arg: %d\n",tmp,  *((int*)tmp->arg));
    }

  p_tpool->cur_queue_size++;
  //fprintf(stdout, "finish add_work\n");
  printf("finish add_work\n");
  pthread_mutex_unlock(&p_tpool->queue_lock);
  return NULL;
}
