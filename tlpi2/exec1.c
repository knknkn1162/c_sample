#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  pid_t pid;

  if((pid = fork()) == -1) {
    perror("fork");
    exit(1);

  } else if (pid == 0) {
    // child
    execve(argv[1], &argv[1], (char**)NULL);
    perror("execve");
    exit(1);
  } else {
    wait(NULL);
    printf("wait parent\n");
    exit(EXIT_SUCCESS);
  }

  return 0;
}
