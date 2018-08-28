#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "msg.h"


int main(int argc, char *argv[]) {
  mqd_t serverMqd, clientMqd;
  char client_queue_name[CLIENT_FIFO_NAME_LEN];
  if((serverMqd = mq_open(SERVER_QUEUE, O_WRONLY)) == (mqd_t)-1) {
    perror("mq_open");
    exit(1);
  }

  snprintf(client_queue_name, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)getpid());

  if((clientMqd = mq_open(client_queue_name, O_RDONLY)) == (mqd_t)-1) {
    perror("mq_open");
    exit(1);
  }


  if(mq_close(serverMqd) == -1) {
    perror("mq_close");
    exit(1);
  }
  if(mq_close(clientMqd) == -1) {
    perror("mq_close");
    exit(1);
  }

  if(mq_unlink(client_queue_name) == -1) {
    perror("mq_unlink");
    exit(1);
  }


  return 0;
}
