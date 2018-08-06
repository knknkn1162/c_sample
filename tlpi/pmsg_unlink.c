#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if(mq_unlink(argv[1]) == -1) {
    perror("mq_unlink");
    exit(1);
  }

  exit(EXIT_SUCCESS);

}
