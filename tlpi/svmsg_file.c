#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <stddef.h>

#define CLIENTS 10
#define SERVER_KEY 0x1aaaaaa1

struct requestMsg {
  long mtype;
  int clientId;
  char pathName[PATH_MAX];
};

#define REQ_MSG_SIZE (offsetof(struct requestMsg, pathName) - offsetof(struct requestMsg, clientId) + PATH_MAX)

#define RESP_MSG_SIZE 8192

struct responseMsg {
  long mtype;
  char data[RESP_MSG_SIZE];
};

#define RESP_MT_FAILURE 1
#define RESP_MT_DATA 2
#define RESP_MT_END 3

static void serveRequest(const struct requestMsg* req);
static void removeQueue(int clientId);

int main(int argc, char* argv[]) {
  struct sigaction sa;
  struct requestMsg req;
  ssize_t msgLen;
  int serverId;

  // parent
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_NOCLDWAIT;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, 0);

  // initialize
  serverId = msgget(SERVER_KEY, 0);
  if(serverId != -1) {
    msgctl(serverId, IPC_RMID, NULL);
  }

  //take this action first!!
  serverId = msgget(SERVER_KEY, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
  if(serverId == -1) {
    perror("msgget");
  }
  
  // client
  for(int i = 0; i < CLIENTS; i++) {
    if(fork() == 0) {
      struct responseMsg resp;
      ssize_t msgLen;
      ssize_t totBytes = 0;
      int clientId;
      int flag = 1;

      // this may fail
      serverId = msgget(SERVER_KEY, S_IWUSR);
      clientId = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | S_IWUSR);

      // do sth
      req.mtype = 1;
      req.clientId = clientId;
      strncpy(req.pathName, argv[1], sizeof(req.pathName) -1);
      req.pathName[sizeof(req.pathName) -1] = '\0';
      sleep(i);
      msgsnd(serverId, &req, REQ_MSG_SIZE, 0);

      /* get response (block?)*/
      printf("wait %d req.pathName: %s, clientid->serverId: %d->%d\n", i, req.pathName, clientId, serverId);
      while(flag) {
        msgLen = msgrcv(clientId, &resp, RESP_MSG_SIZE, 0, 0);
        if(msgLen == -1) {
          printf("msgLen == -1\n");
          if(errno == EINTR) { /* Interrupted by SIGCHLD handler? */
            continue;
          }
          perror("msg");
          break;
        }
        switch(resp.mtype) {
          case RESP_MT_FAILURE:
            msgctl(clientId, IPC_RMID, NULL);
            printf("%d failure\n", i);
            _exit(EXIT_FAILURE);
            break;
          case RESP_MT_DATA:
            totBytes += msgLen;
            break;
          case RESP_MT_END:
            flag = 0;
            printf("%d end\n", i);
            break;
        }
      }
      printf("bytes: %ld\n---------\n", totBytes);

      removeQueue(clientId);
      _exit(EXIT_SUCCESS);
    }
  }

  while(1) {
    // ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
    msgLen = msgrcv(serverId, &req, REQ_MSG_SIZE, 0, 0);
    printf("[server]receive: %s[%d]\n", req.pathName, req.clientId);
    switch(fork()) {
      case -1:
        perror("fork");
        _exit(EXIT_SUCCESS);
      case 0:
        serveRequest(&req);
        _exit(EXIT_SUCCESS);
    }
  }
  msgctl(serverId, IPC_RMID, NULL);
  exit(EXIT_SUCCESS);

  return 0;
}

static void serveRequest(const struct requestMsg *req) {
  int fd;
  ssize_t numRead;
  struct responseMsg resp;

  fd = open(req->pathName, O_RDONLY | O_CLOEXEC);
  if(fd == -1) {
    resp.mtype = RESP_MT_FAILURE;
    snprintf(resp.data, sizeof(resp.data), "%s", "couldnt open");
    msgsnd(req->clientId, &resp, strlen(resp.data) + 1, 0);
    perror("serveRequest open");
    exit(EXIT_FAILURE);
  }
  printf("serveRequest start[id: %d]\n", req->clientId);;

  resp.mtype = RESP_MT_DATA;
  while((numRead = read(fd, resp.data, RESP_MSG_SIZE)) > 0) {
    printf("msgsnd %ld\n", numRead);
    if(msgsnd(req->clientId, &resp, numRead, 0) == -1) { break; }
  }

  resp.mtype = RESP_MT_END;
  msgsnd(req->clientId, &resp, 0, 0);
  printf("serveRequest end[id: %d]\n", req->clientId);
}

static void removeQueue(int clientId) {
  msgctl(clientId, IPC_RMID, NULL);
}
