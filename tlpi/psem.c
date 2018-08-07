#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
  sem_t *sem;

  sem = sem_open(argv[1], O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
  system("ls /dev/shm");
  //sem_close(sem);
  if(fork() == 0) {

    printf("post\n");
    if(sem_post(sem) == -1) {
      perror("sem_post");
    }
    _exit(EXIT_SUCCESS);
  } else {

    printf("wait\n");
    if(sem_wait(sem) == -1) {
      perror("sem_wait");
    }
    printf("wake up\n");
  }
  sem_unlink(argv[1]);
}
