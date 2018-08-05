#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BUF_SIZE 1024


union semun {
  int              val;    /* SETVAL の値 */
  struct semid_ds *buf;    /* IPC_STAT, IPC_SET 用のバッファー */
  unsigned short  *array;  /* GETALL, SETALL 用の配列 */
  struct seminfo  *__buf;  /* IPC_INFO 用のバッファー */
};

int main(int argc, char *argv[]) {
  int semid;
  if(argc == 2) {
    union semun arg;

    // int semget(key_t key, int nsems, int semflg)
    semid = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
    arg.val = atoi(argv[1]);
    if(semctl(semid, 0, SETVAL, arg) == -1) {
      perror("semctl");
      exit(1);
    }
    printf("semaphore ID = %d\n", semid);
  } else {
    struct sembuf sop;
    semid = atoi(argv[1]);

    sop.sem_num = 0;
    sop.sem_op = atoi(argv[2]);
    sop.sem_flg = 0;

    printf("PID: %d\n", getpid());
    semop(semid, &sop, 1);

    printf("PID: %d\n", getpid());
  }
  return 0;
}
