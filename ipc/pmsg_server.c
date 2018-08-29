#include <mqueue.h>
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define NOTIFY_SIG SIGUSR1

int main(int argc, char *argv[]) {
  mqd_t serverMqd, clientMqd;
  struct mq_attr attr;
  char *buf;
  unsigned int prio;
  struct sigevent sev;
  sigset_t blockMask, emptyMask;
  char client_queue_name[CLIENT_FIFO_NAME_LEN];

  if((serverMqd = mq_open(SERVER_QUEUE, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) == (mqd_t)-1) {
    perror("[server]mq_open server");
    exit(1);
  }

  if(mq_getattr(serverMqd, &attr) == -1) {
    perror("mq_getattr");
    exit(1);
  }

  printf("maxmsg: %ld, msgsize: %ld, cur_msgs: %ld\n", attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
  buf = malloc(attr.mq_msgsize);
  if(buf == NULL) {
    perror("malloc");
    exit(1);
  }

  sigemptyset(&blockMask);
  sigaddset(&blockMask, NOTIFY_SIG);
  if(sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = NOTIFY_SIG;
  // register the process to receive message notification
  if(mq_notify(serverMqd, &sev) == -1) {
    perror("mq_notify");
    exit(1);
  }

  while(1) {
    // wait for notification signal
    siginfo_t info;

    if(sigwaitinfo(&blockMask, &info) == -1) {
      perror("sigwaitinfo");
    }

    // reregister the process to receive message notification
    if(mq_notify(serverMqd, &sev) == -1) {
      perror("mq_notify reregister");
    }

    int numRead = mq_receive(serverMqd, buf, attr.mq_msgsize, &prio);
    if(numRead == -1) {
      perror("mq_receive");
      exit(1);
    }

    // retrieve pid
    snprintf(client_queue_name, CLIENT_QUEUE_NAME_LEN, CLIENT_QUEUE_TEMPLATE, (long)info.si_pid);
    printf("[server] client_queue: %s\n", client_queue_name);
    if((clientMqd = mq_open(client_queue_name, O_WRONLY)) == (mqd_t)-1) {
      perror("[server]mq_open client");
      exit(1);
    }

    if(mq_send(clientMqd, buf, numRead, 0) == -1) {
      perror("[server] mq_send");
      exit(1);
    }
    if(mq_close(clientMqd) == -1) {
      perror("mq_close");
      exit(1);
    }
  }


  if(mq_close(serverMqd) == -1) {
    perror("mq_close");
    exit(1);
  }
  if(mq_unlink(SERVER_QUEUE) == -1) {
    perror("mq_unlink");
    exit(1);
  }
  return 0;
}
