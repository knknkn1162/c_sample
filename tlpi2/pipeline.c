#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
  pid_t pid;
  if((pid = fork()) == -1) {
    perror("fork");
    exit(1);
    // child
  } else if(pid == 0) {

  } else {
    // parent
  }

}
