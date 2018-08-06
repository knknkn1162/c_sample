#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define NOTIFY_SIG SIGUSR1

static void handler(int sig) {}

int main(int argc, char *argv[]) {
  struct sigevent sev;
  mqd_t mqd;
  struct mq_attr attr;
  char *buffer;
  ssize_t numRead;
  sigset_t blockMask, emptyMask;
  struct sigaction sa;

  mqd = mq_open(argv[1], O_CREAT | O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL);
  if(mqd == (mqd_t)-1) {
    perror("mq_open");
    exit(1);
  }

  if(mq_getattr(mqd, &attr) == -1) {
    perror("mq_getattr");
    exit(1);
  }

  buffer = (char*)malloc(attr.mq_msgsize);
  if(buffer == NULL) {
    perror("malloc");
    exit(1);
  }

  // block NOTIFY_SIG to establish handler
  sigemptyset(&blockMask);
  sigaddset(&blockMask, NOTIFY_SIG);
  if(sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
    perror("sigprocmask");
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(NOTIFY_SIG, &sa, NULL) == -1) {
    perror("sigaction");
  }

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = NOTIFY_SIG;
  if(mq_notify(mqd, &sev) == -1) {
    perror("mq_notify");
  }

  sigemptyset(&emptyMask);

  printf("start\n");
  if(fork() == 0) {
    int i;
    for(i = 0; i < 5; i++) {
      char buf[256];
      snprintf(buf, 256, "[%d] %s\n", i, argv[2]);
      mqd = mq_open(argv[1], O_WRONLY);
      // send
      if(mq_send(mqd, buf, strlen(buf), 0) == -1) {
        perror("mq_send");
        exit(1);
      }
      printf("child send messages\n");
      sleep(1);
    }
    _exit(EXIT_SUCCESS);
  }

  while(1) {
    printf("before sigsuspend\n");
    sigsuspend(&emptyMask);
    if(mq_notify(mqd, &sev) == -1) {
      perror("mq_notify");
    }

    while((numRead = mq_receive(mqd, buffer, attr.mq_msgsize, NULL)) > 0) {
      printf("Read %ld bytes .. %s\n", (long)numRead, buffer);
    }
    if(errno != EAGAIN) {
      perror("mq_receive");
    }
  }
}
