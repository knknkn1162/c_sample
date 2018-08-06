#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

static void threadFunc(union sigval sv);

int main(int argc, char *argv[]) {
  mqd_t mqd;
  struct sigevent sev;
  mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
  if(mqd == (mqd_t)-1) {
    perror("mq_open");
    exit(1);
  }
/* struct sigevent { */
    /* int          sigev_notify; [> Notification method <] */
    /* int          sigev_signo;  [> Notification signal <] */
    /* union sigval sigev_value;  * Data passed with */
                                  /* notification */
    /* void       (*sigev_notify_function) (union sigval); */
                     /* * Function used for thread */
                        /* notification (SIGEV_THREAD) */
    /* void        *sigev_notify_attributes; */
                     /* * Attributes for notification thread */
                        /* (SIGEV_THREAD) */ 
    /* pid_t        sigev_notify_thread_id; */
                     /* [> ID of thread to signal (SIGEV_THREAD_ID) <] */
  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = threadFunc;
  sev.sigev_notify_attributes = NULL;
  sev.sigev_value.sival_ptr = &mqd;
  if(mq_notify(mqd, &sev) == -1) {
    perror("mq_notify");
  }

  {
    mqd_t mqd;
    mqd = mq_open(argv[1], O_WRONLY);
    // send
    if(mq_send(mqd, argv[2], strlen(argv[2]), 0) == -1) {
      perror("mq_send");
      exit(1);
    }
    printf("send word\n");
  }
  pause();
}

/* union sigval {          [> Data passed with notification <] */
    /* int     sival_int;         [> Integer value <] */
    /* void   *sival_ptr;         [> Pointer value <] */
/* }; */
static void threadFunc(union sigval sv) {

  ssize_t numRead;
  mqd_t *mqdp;
  char *buffer;
  struct mq_attr attr;
  struct sigevent sev;

  printf("threadfunc start\n");
  mqdp = sv.sival_ptr;
  if(mq_getattr(*mqdp, &attr) == -1) {
    perror("mq_getattr");
    exit(1);
  }

  buffer = (char*)malloc(attr.mq_msgsize);
  if(buffer == NULL) {
    perror("malloc");
  }

  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = threadFunc;
  sev.sigev_notify_attributes = NULL;
  sev.sigev_value.sival_ptr = &mqdp;
  if(mq_notify(*mqdp, &sev) == -1) {
    perror("mq_notify");
  }

  while((numRead = mq_receive(*mqdp, buffer, attr.mq_msgsize, NULL)) >= 0) {
    printf("Read %ld bytes [%s]\n", (long)numRead, buffer);
  }

  if(errno != EAGAIN) {
    perror("mq_receive");
    exit(1);
  }

  free(buffer);
  pthread_exit(NULL);
}
