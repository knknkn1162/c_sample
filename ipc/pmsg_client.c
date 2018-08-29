#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "msg.h"
#include <string.h>

char *buf;
void cleanup_handler(void) {
  free(buf);
}

int main(int argc, char *argv[]) {
  mqd_t serverMqd, clientMqd;
  struct mq_attr serverAttr, clientAttr;
  unsigned int prio = 0;
  int num;

  char client_queue_name[CLIENT_FIFO_NAME_LEN];

  if(argc <= 1) {
    fprintf(stderr, "usage error\n");
    exit(1);
  }

  if((serverMqd = mq_open(SERVER_QUEUE, O_WRONLY)) == (mqd_t)-1) {
    perror("mq_open server");
    exit(1);
  }
  // for display only
  if(mq_getattr(serverMqd, &serverAttr) == -1) {
    perror("mq_getAttr");
    exit(1);
  }

  snprintf(client_queue_name, CLIENT_QUEUE_NAME_LEN, CLIENT_QUEUE_TEMPLATE, (long)getpid());

  printf("client queue: %s\n", client_queue_name);
  if((clientMqd = mq_open(client_queue_name, O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR, NULL)) == (mqd_t)-1) {
    perror("mq_open client");
    exit(1);
  }

  // to get maximum of message size of queue.
  if(mq_getattr(clientMqd, &clientAttr) == -1) {
    perror("mq_getattr");
    exit(1);
  }
  buf = malloc(clientAttr.mq_msgsize);
  atexit(cleanup_handler);

  // send message
  // block when the queue size > cur_msgs
  if(mq_send(serverMqd, argv[1], strlen(argv[1]), 0) == -1) {
    perror("[client] mq_send");
    exit(1);
  }

  // no longer use
  if(mq_close(serverMqd) == -1) {
    perror("mq_close");
    exit(1);
  }

  printf("maxmsg: %ld, msgsize: %ld, cur_msgs: %ld\n", serverAttr.mq_maxmsg, serverAttr.mq_msgsize, serverAttr.mq_curmsgs);

  // receive message
  if((num = mq_receive(clientMqd, buf, clientAttr.mq_msgsize, &prio)) == -1) {
    perror("mq_receive");
    exit(1);
  }

  if(write(STDOUT_FILENO, buf, num) != num) {
    perror("write");
    exit(1);
  }
  write(STDOUT_FILENO, "\n", 1);

  printf("[client]close client message queue\n");
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
