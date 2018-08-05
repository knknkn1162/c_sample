
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

union semun {
  int              val;    /* SETVAL の値 */
  struct semid_ds *buf;    /* IPC_STAT, IPC_SET 用のバッファー */
  unsigned short  *array;  /* GETALL, SETALL 用の配列 */
  struct seminfo  *__buf;  /* IPC_INFO 用のバッファー */
};

int main(int argc, char *argv[]) {

  key_t key = 0x12;
  int semid;
  semid = semget(key, 1, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  if(semid != -1) {
    printf("semid: %d\n", semid);
    // assume to get high pressure
    sleep(5);
    union semun arg;
    struct sembuf sop;

    arg.val = 0;
    if(semctl(semid, 0, SETVAL, arg) == -1) {
      perror("semctl");
      exit(1);
    }

    // notify initialization by no-op semaphore
    sop.sem_num = 0;
    sop.sem_op = 0;
    sop.sem_flg = 0;
    if(semop(semid, &sop, 1) == 1) {
      perror("semop");
      exit(1);
    }
    printf("initialized!\n");
  } else {
    const int MAX_TRIES = 10;
    int j;
    union semun arg;
    struct semid_ds ds;

    if(errno != EEXIST) {
      perror("semget");
      exit(1);
    }

    semid = semget(key, 1, S_IRUSR | S_IWUSR);

    arg.buf = &ds;
    for(j = 0; j < MAX_TRIES; j++) {
      if(semctl(semid, 0, IPC_STAT, arg) == -1) {
        perror("semctl");
        exit(1);
      }
      /* ensure that the semaphore really initializes */
      if(ds.sem_otime != 0) { break; }
      sleep(1);
    }

    if(ds.sem_otime == 0) {
      perror("not initialized");
      exit(1);
    }

    printf("now perform some op\n");
  }
}
