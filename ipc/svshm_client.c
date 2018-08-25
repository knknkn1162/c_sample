#include "msg.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "binary_sems.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

  int semid, shmid;
  struct response *resp;

  semid = semget(SEM_KEY, 0, 0);
  if(semid == -1) {
    perror("semget");
    exit(1);
  }

  shmid = shmget(SHM_KEY, 0, 0);
  if(shmid == -1) {
    perror("shmget");
    exit(1);
  }

  resp = shmat(shmid, NULL, SHM_RDONLY);
  if(resp == (void*)-1) {
    perror("shmat");
    exit(1);
  }

  int flag = 1;
  while(flag) {
    if(reserveSem(semid, READ_SEM) == -1) {
      perror("reserveSem");
      exit(1);
    }

    switch(resp->mtype) {
      case RESP_DATA:
        printf("[client(%d)]> %s\n", getpid(), resp->message);
        break;
      case RESP_END:
        printf("[client(%d)]> end\n", getpid());
        flag = 0;
        break;
      case RESP_FAILURE:
        fprintf(stderr, "[client(%d)] ERROR: %s\n", getpid(), resp->message);
        flag = 0;
        break;
    }

    if(releaseSem(semid, WRITE_SEM) == -1) {
      perror("releaseSem");
      exit(1);
    }
  }

  if(shmdt(resp) == -1) {
    perror("shmdt");
    exit(1);
  }

  return 0;
}
