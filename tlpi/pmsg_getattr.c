#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  mqd_t mqd;
  struct mq_attr attr;

  mqd = mq_open(argv[1], O_RDONLY);
  if(mqd == (mqd_t)-1) {
    perror("mq_open");
    exit(1);
  }

  if(mq_getattr(mqd, &attr) == -1) {
    perror("mq_getattr");
    exit(1);
  }

  printf("maximum # of messages on queue %ld\n", attr.mq_maxmsg);
  printf("maximum message size %ld\n", attr.mq_msgsize);
  printf("# of messages currently on queue %ld\n", attr.mq_curmsgs);
  exit(EXIT_SUCCESS);
}
