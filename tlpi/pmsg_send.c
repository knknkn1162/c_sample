#include <mqueue.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  mqd_t mqd;
  mqd = mq_open(argv[1], O_WRONLY);
  if(mqd == (mqd_t)-1) {
    perror("mq_open");
  }

  if(mq_send(mqd, argv[2], strlen(argv[2]), 0) == -1) {
    perror("mq_send");
    exit(1);
  }
}
