#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if(fork() == 0) {
    execlp("wc", "wc", "-l", (char*)NULL);
  }
  /* char buf[1024]; */
  /* read(STDIN_FILENO, buf, 1024); */
  sleep(3);
  close(STDOUT_FILENO);
  return 0;
}
