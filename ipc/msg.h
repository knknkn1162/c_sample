#include <limits.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#define SERVER_KEY 0x1aaaaaa1
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678


#define SERVER_FIFO "/tmp/seqnum_sv"
#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)


#define REQ_MSG_SIZE PATH_MAX
struct request {
#ifdef SYSTEM_V_MESSAGE
  long mtype;
#endif
  long clientId;
  char pathName[REQ_MSG_SIZE];
};
#define REQ_SIZE (sizeof(struct request))

#define RESP_MSG_SIZE 256
#define RESP_SIZE (sizeof(struct response))

struct response {
  long mtype;
  char message[RESP_MSG_SIZE];
};

#define RESP_FAILURE 1
#define RESP_DATA 2
#define RESP_END 3

#ifdef SYSTEM_V_MESSAGE
#define REQ_BODY_SIZE sizeof(struct request) - sizeof(long)
#define RESP_BODY_SIZE RESP_MSG_SIZE

int rm_message_queue(int msqid) {
  return msgctl(msqid, IPC_RMID, NULL);
}
void exit_with_message_queue(int msqid, char* errmsg) {
  int savedErrno = errno;
  perror(errmsg);
  rm_message_queue(msqid);
  errno = savedErrno;
  abort();
}
#endif
