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
  struct response *shmp;
  //union semun dummy;
  int fd;

  /* semid = semget(SEM_KEY, 2, IPC_CREAT | S_IRUSR | S_IWUSR); */
  /* if(semid == -1) { */
    /* perror("exit"); */
  /* } */

  // create a new shared memory segment
  shmid = shmget(SHM_KEY, sizeof(struct response), IPC_CREAT | S_IRUSR | S_IWUSR);
  if(shmid == -1) {
    perror("shmget");
    exit(1);
  }

  // attaches the shared memory segment
  shmp = shmat(shmid, NULL, 0);
  if(shmp == (void*)-1) {
    perror("shmat");
  }

  if((fd = open(argv[1], O_RDONLY)) == -1) {
    perror("open");
    exit(1);
  }
  while(1) {
    int numRead;
    // reserve shared memory

    if((numRead = read(fd, shmp->message, RESP_MSG_SIZE)) == -1) {
      perror("read");
      exit(1);
    } else if(numRead == 0) {
      shmp->mtype = RESP_END;
      printf("[server] end\n");
    } else {
      shmp->mtype = RESP_DATA;
      printf("[server] > %s\n", shmp->message);
    }

    // release shared memory

    if(numRead == 0) {
      break;
    }
  }

  // remove semaphore
  /* if(semctl(semid, 0, IPC_RMID, dummy) == -1) { */
    /* perror("semctl"); */
    /* exit(1); */
  /* } */

  // detach and remove shared memory
  if(shmdt(shmp) == -1) {
    perror("shmdt");
    exit(1);
  }
  if(shmctl(shmid, IPC_RMID, 0) == -1) {
    perror("shmctl");
    exit(1);
  }

  return 0;
}


