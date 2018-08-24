#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "sv_msg.h"

int main(int argc, char *argv[]) {
  int msqid;

  msqid = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);
  if(msqid == -1) {
    perror("msgget");
  }

  while(1) {
    int msgLen;
    struct request req;
    struct response res;
    // ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
    if((msgLen = msgrcv(msqid, &req, REQ_MSG_SIZE, 0, 0)) == -1) {
        perror("msgrcv");
    }

    // echo
    strncpy(res.mtext, req.mtext, RESP_MSG_SIZE);
    if(msgsnd(msqid, &res, msgLen, 0) == -1) {
      perror("msgsnd");
      exit(1);
    }
  }
  return 0;
}
