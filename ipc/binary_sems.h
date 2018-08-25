#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>

int bsUseSemUndo = 0;
int bsRetryOnEintr = 1; 

 union semun {                   /* Used in calls to semctl() */
     int                 val;
     struct semid_ds *   buf;
     unsigned short *    array;
 #if defined(__linux__)
     struct seminfo *    __buf;
 #endif
 };

int initSemAvailable(int semId, int semNum)
{
    union semun arg;

    arg.val = 1;
    return semctl(semId, semNum, SETVAL, arg);
}

int initSemInUse(int semId, int semNum)
{
  union semun arg;

  arg.val = 0;
  return semctl(semId, semNum, SETVAL, arg);
}

int reserveSem(int semId, int semNum)
{
  struct sembuf sops;

  sops.sem_num = semNum;
  sops.sem_op = -1;
  sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

  while (semop(semId, &sops, 1) == -1)
      if (errno != EINTR || !bsRetryOnEintr)
          return -1;

  return 0;
}

int releaseSem(int semId, int semNum)
{
  struct sembuf sops;

  sops.sem_num = semNum;
  sops.sem_op = 1;
  sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

  return semop(semId, &sops, 1);
}
