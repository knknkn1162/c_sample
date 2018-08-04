#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_MTEXT 1024


struct mbuf {
  long mtype;
  char mtext[MAX_MTEXT];
};

int main(int argc, char* argv[]) {
  key_t key;
  struct mbuf msg;
  struct mbuf recv_msg;
  int msgLen;
  int type = -30;
  char* str = "abcd";

  key = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);

  printf("key: %d created\n", key);

  switch(fork()) {
    case 0:
      msgLen = strlen(str) + 1;
      printf("length: %d\n", msgLen);
      memcpy(msg.mtext, str, msgLen);
      msg.mtype = 20;
      msgsnd(key, &msg, msgLen, 0);
      _exit(EXIT_SUCCESS);
    default:
      msgrcv(key, &recv_msg, MAX_MTEXT, type, IPC_NOWAIT);
      if(errno == ENOMSG) {
        printf("(no msg)\n");
      } else {
        printf("msg.mtext: %s\n", recv_msg.mtext);
      }
      wait(NULL);
      break;
  }
  printf("+ before\n");
  system("ipcs");
  msgctl(key, IPC_RMID, NULL);
  printf("+ after\n");
  system("ipcs");
  return 0;
}
