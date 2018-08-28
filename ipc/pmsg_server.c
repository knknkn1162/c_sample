#include <mqueue.h>
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  mqd_t mqd;
  struct mq_attr attr;
  char buf[RESP_BUF_SIZE];

  if((mqd = mq_open(SERVER_QUEUE, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR)) == (mqd_t)-1) {
    perror("mq_open");
    exit(1);
  }

  if(mq_getattr(mqd, &attr) == -1) {
    perror("mq_getattr");
    exit(1);
  }

  printf("maxmsg: %ld, msgsize: %ld, cur_msgs: %ld\n", attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);

  /* while(1) { */
    int numRead = mq_recv(mqd, buf, BUF_SIZE, NULL);
    if(write(STDOUT_FILENO, buf, numRead) == -1) {
      perror("write");
      exit(1);
    }
  /* } */


  if(mq_close(mqd) == -1) {
    perror("mq_close");
    exit(1);
  }
  if(mq_unlink(SERVER_QUEUE) == -1) {
    perror("mq_unlink");
    exit(1);
  }
  return 0;
}
