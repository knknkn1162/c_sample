#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define KEY_FILE "./Dockerfile"

int main(void) {
  int msqid;
  key_t key;
  key = ftok(KEY_FILE, 1);
  printf("0x%08x\n", key);
  
  while((msqid = msgget(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP)) == -1) {
    if(errno == EEXIST) {
      msqid = msgget(key, 0);
      msgctl(msqid, IPC_RMID, NULL);
      printf("%d\n", msqid); 
    } else {
      perror("EExit");
      exit(1);
    }
  }

  exit(EXIT_SUCCESS);
}
