#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#define SYSTEM_V_MESSAGE
#include "msg.h"

int main(int argc, char *argv[]) {
  int serverId;

  serverId = msgget(SERVER_KEY, IPC_CREAT | S_IRUSR | S_IWUSR);
  if(serverId == -1) {
    perror("msgget");
  }
  printf("[server] create System V key\n");

  while(1) {
    int msgLen;
    struct request req;
    struct response resp;
    // ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
    if((msgLen = msgrcv(serverId, &req, REQ_MSG_SIZE, 0, 0)) == -1) {
      exit_with_message_queue(serverId, "msgrcv");
    }
    printf("[server] receive message with %ld\n", req.clientId);

    // echo
    resp.mtype = RESP_DATA;
    strncpy(resp.message, req.pathName, strlen(req.pathName));
    if(msgsnd(req.clientId, &resp, sizeof(resp) - sizeof(long), 0) == -1) {
      exit_with_message_queue(serverId, "msgsnd");
    }
    printf("[server] send message to %ld\n", req.clientId);
  }
  return 0;
}
