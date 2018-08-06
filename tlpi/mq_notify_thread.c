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
  mqd_t mqd2;
  struct sigevent sev;

  /* initialize */
  mqd = mq_open(argv[1], O_CREAT, S_IRUSR | S_IWUSR, NULL);
  memset(&sev, 0, sizeof(sev));
  if(mqd == (mqd_t)-1) {
    perror("open");
    exit(1);
  }
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

    int i;
    mqd2 = mq_open(argv[1], O_WRONLY);
    // send multiple times
    for(i = 0; i < 5; i++) {
      // send
      sleep(1);
      if(mq_send(mqd2, argv[2], strlen(argv[2]), 0) == -1) {
        perror("mq_send");
        exit(1);
      }
      printf("send word [%d]\n", i);
    }
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

  write(STDOUT_FILENO, "threadfunc start\n", 17);
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
  sev.sigev_value.sival_ptr = mqdp;
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
