#include "svshm_xfr.h"
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

union semun {
  int              val;    /* SETVAL の値 */
  struct semid_ds *buf;    /* IPC_STAT, IPC_SET 用のバッファー */
  unsigned short  *array;  /* GETALL, SETALL 用の配列 */
  struct seminfo  *__buf;  /* IPC_INFO 用のバッファー */
};

int bsUseSemUndo = 0;
int bsRetryOnEintr = 1;
int initSemAvailable(int semId, int semNum);
int initSemInUse(int semId, int semNum);
int reserveSem(int semId, int semNum);
int releaseSem(int semId, int semNum);

int main(int argc, char *argv[]) {
  int semid, shmid;
  struct shmseg *shmp;
  union semun dummy;
  int j = 0;
  int bytes = 0;

  // create semaphore
  semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);
  initSemAvailable(semid, WRITE_SEM);
  initSemInUse(semid, READ_SEM);
  shmid = shmget(SHM_KEY, sizeof(struct shmseg), IPC_CREAT | OBJ_PERMS);

  if(shmid == -1) {
    perror("semget");
    exit(1);
  }

  if(fork() == 0) {
    // reader
    int fd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, 0600);
    if(fd == -1) {
      perror("fd open for wdonly");
      exit(1);
    }
    // Don't confuse semget and shmget!!
    semid = semget(SEM_KEY, 0, 0);
    shmid = shmget(SHM_KEY, 0, 0);
    shmp = shmat(shmid, NULL, SHM_RDONLY);
    printf("[reader]shmp: %p\n", shmp);
    while(1) {
      // reader
      if(reserveSem(semid, READ_SEM) == -1) {
        perror("reserve READ_SEM for reader");
        exit(1);
      }
      printf("reserve READ_SEM\n");
      // if EOF, break
      if(shmp->cnt == 0) break;
      bytes += shmp->cnt;
      printf("write in %s [~%d bytes]\n", argv[2], bytes);
      if(write(fd, shmp->buf, shmp->cnt) == -1) {
        perror("write");
        exit(1);
      }

      if(releaseSem(semid, WRITE_SEM) == -1) {
        perror("releaseSem for reader");
        exit(1);
      }
      printf("release WRITE_SEM for reader\n");
      j++;
    }
    shmdt(shmp);
    /* release WRITE_SEM after break */
    releaseSem(semid, WRITE_SEM);
    printf("release WRITE_SEM for reader and exit!");
    fprintf(stderr, "Received %d bytes, (%d xfrs)\n", bytes, j);
    _exit(EXIT_SUCCESS);
  } else {
    // writer
    int fd = open(argv[1], O_RDONLY | O_CLOEXEC);
    // void *shmat(int shmid, const void *shmaddr, int shmflg);
    shmp = shmat(shmid, NULL, 0);
    printf("[writer]shmp: %p\n", shmp);
    if(shmp == (void*)-1) {
      perror("shmp -1");
      exit(1);
    }
    while(1) {
      /* may block */
      if(reserveSem(semid, WRITE_SEM) == -1) {
        perror("reserveSem");
        exit(1);
      }

      shmp->cnt = read(fd, shmp->buf, BUF_SIZE);
      
      printf("read %d bytes\n", shmp->cnt);
      if(releaseSem(semid, READ_SEM) == -1) {
        perror("releaseSem");
        exit(1);
      }
      printf("release read_sem\n");
      if(shmp->cnt == 0) { break; }
      bytes += shmp->cnt;
      j++;
    }
    // detach
    shmdt(shmp);
    printf("sent %d bytes (%d xfrs)\n", bytes, j);
  }

  reserveSem(semid, WRITE_SEM);
  // destroy
  // int semctl(int semid, int semnum, int cmd, ...);
  // When cmd = IPC_RMID, semnum is avoided.
  semctl(semid, 0, IPC_RMID, dummy);
  shmdt(shmp);
  shmctl(shmid, IPC_RMID, 0);
  exit(EXIT_SUCCESS);

  return 0;
}

int initSemAvailable(int semId, int semNum) {
  union semun arg;
  arg.val = 1;
  return semctl(semId, semNum, SETVAL, arg);
}

int initSemInUse(int semId, int semNum) {
  union semun arg;
  arg.val = 0;
  return semctl(semId, semNum, SETVAL, arg);
}

int reserveSem(int semId, int semNum) {
  struct sembuf sops;
  sops.sem_num = semNum;
  sops.sem_op = -1;
  sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

  while(semop(semId, &sops, 1) == -1) {
    if(errno != EINTR || !bsRetryOnEintr) {
      return -1;
    }
  }

  return 0;
}

int releaseSem(int semId, int semNum) {
  struct sembuf sops;
  sops.sem_num = semNum;
  sops.sem_op = 1;
  sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

  return semop(semId, &sops, 1);
}
