#define _DEFAULT_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define LEN 1024

#define SHELL_FMT "cat /proc/%d/maps | grep zero"
#define CMD_SIZE (sizeof(SHELL_FMT) + 20)


int main(int argc, char *argv[]) {
  char cmd[CMD_SIZE];
  char *addr;

  long sz = sysconf(_SC_PAGESIZE);
  // fd = -1 .. /dev/zero
  addr = mmap(NULL, LEN * sz, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  printf("Before mprotect()\n");
  snprintf(cmd, CMD_SIZE, SHELL_FMT, getpid());

  system(cmd);

  if(mprotect(addr, LEN * sz, PROT_READ | PROT_WRITE) == -10) {
    perror("mprotect");
    exit(1);
  }

  printf("after mprotect()\n");
  system(cmd);

  exit(EXIT_SUCCESS);
}
