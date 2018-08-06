#ifdef USE_MAP_ANON
#define _DEFAULT_SOURCE
#endif
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int *addr;
// compile `gcc -DUSE_MAP_ANON tlpi/anon_map.c`
#ifdef USE_MAP_ANON
  addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
#else
  int fd;
  fd = open("/dev/zero", O_RDWR);
  if(fd == -1) {
    perror("open");
    exit(1);
  }

  addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  if(close(fd) == -1) {
    perror("close");
  }
#endif

  printf("initial addr %d\n", *addr);
  *addr = 1;
  switch(fork()) {
    case -1:
      perror("fork");
      _exit(1);
    case 0:
      printf("child started %d\n", *addr);
      (*addr)++;
      if(munmap(addr, sizeof(int)) == -1) {
        perror("munmap");
        exit(1);
      }
      _exit(EXIT_SUCCESS);
    default:
      // sync
      if(wait(NULL) == -1) {
        perror("wait");
      }
      printf("In parent value = %d\n", *addr);
      if(munmap(addr, sizeof(int)) == -1) {
        perror("parent munmap");
        exit(1);
      }
      exit(EXIT_SUCCESS);
  }
}
