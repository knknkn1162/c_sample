#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define SYSTEM_V_MESSAGE
#include "msg.h"

static int clientId;
void removeQueue() {
  if(rm_message_queue(clientId) == -1) {
    perror("rm_message_queue");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  int serverId;
  struct request req;
  struct response resp;

  if(argc < 2) {
    fprintf(stderr, "usage error\n");
    exit(1);
  }

  serverId = msgget(SERVER_KEY, S_IWUSR);
  if(serverId == -1) {
    perror("[client] msgget");
    fprintf(stderr, "Maybe the server has not started yet!\n");
    exit(1);
  }

  clientId = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | S_IWGRP);
  if(clientId == -1) {
    perror("msgget");
    exit(1);
  }

  if(atexit(removeQueue) != 0) {
    perror("atexit");
    exit(1);
  }

  memset(&req, 0, sizeof(req));
  // The server detects where to send message.
  req.clientId = clientId;
  req.mtype = 1;
  strncpy(req.pathName, argv[1], sizeof(req.pathName) - 1);

  // int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
  if(msgsnd(serverId, &req, REQ_MSG_SIZE, 0) == -1) {
    exit_with_message_queue(clientId, "msgsnd");
  }
  printf("[client] send message to %d\n", clientId);

  // ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
  if(msgrcv(clientId, &resp, RESP_BODY_SIZE, 0, 0) == -1) {
    printf("%s\n", resp.message);
    exit_with_message_queue(clientId, "msgrcv");
  }
  printf("[client] receive message with server\n");

  switch(resp.mtype) {
    case RESP_DATA:
      printf("[client(%d)]> %s\n", getpid(), resp.message);
      break;
    case RESP_END:
      printf("[client(%d)]> end\n", getpid());
      break;
    case RESP_FAILURE:
      fprintf(stderr, "[client(%d)] ERROR: %s\n", getpid(), resp.message);
      break;
  }
  return 0;
}
