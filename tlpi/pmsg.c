#include <mqueue.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
  mqd_t mqd;
  // create
  mqd = mq_open(argv[1], O_CREAT, S_IRUSR | S_IWUSR, NULL);
  if(mqd == (mqd_t)-1) {
    perror("open");
    exit(1);
  }

  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      mqd = mq_open(argv[1], O_WRONLY);
      // send
      if(mq_send(mqd, argv[2], strlen(argv[2]), 0) == -1) {
        perror("mq_send");
        exit(1);
      }
      printf("child send messages\n");
      _exit(EXIT_SUCCESS);
    default: {
      struct mq_attr attr;
      char *buf;
      int numRead;
      mqd = mq_open(argv[1], O_RDONLY);
      if(wait(NULL) == -1) {
        perror("wait");
        exit(1);
      }
      mq_getattr(mqd, &attr);
      buf = malloc(attr.mq_msgsize);
      numRead = mq_receive(mqd, buf, attr.mq_msgsize, NULL);
      if(numRead == -1) {
        perror("mq_receive");
        exit(1);
      }
      printf("receive %ld bytes\n", (long)numRead);
      if(write(STDOUT_FILENO, buf, numRead) == -1) {
        perror("write");
        exit(1);
      }
      putchar('\n');
      break;
    }
  }

  if(mq_unlink(argv[1]) == -1) {
    perror("mq_unlink");
    exit(1);
  }
  printf("mq_unlink\n");
  exit(EXIT_SUCCESS);
}
