#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

union semun {
  int              val;    /* SETVAL の値 */
  struct semid_ds *buf;    /* IPC_STAT, IPC_SET 用のバッファー */
  unsigned short  *array;  /* GETALL, SETALL 用の配列 */
  struct seminfo  *__buf;  /* IPC_INFO 用のバッファー */
};

int main(int argc, char *argv[]) {
  struct semid_ds ds;
  union semun arg, dummy;
  int semid, j;

  semid = atoi(argv[1]);
  arg.buf = &ds;
  semctl(semid, 0, IPC_STAT, arg);

  printf("Semaphore changed %s", ctime(&ds.sem_ctime));
  printf("Semaphore changed %s", ctime(&ds.sem_otime));


  arg.array = calloc(ds.sem_nsems, sizeof(arg.array[0]));
  semctl(semid, 0, GETALL, arg);

  for(j = 0; j < ds.sem_nsems; j++) {
    printf("%d %d %d %d %d\n", j, arg.array[j],
        semctl(semid, j, GETPID, dummy),
        semctl(semid, j, GETNCNT, dummy),
        semctl(semid, j, GETZCNT, dummy)
    );
  }
  exit(EXIT_SUCCESS);
}
