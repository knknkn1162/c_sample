#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MEM_SIZE 10

int main(int argc, char *argv[]) {

  char *addr;
  int fd;

  fd = open(argv[1], O_RDWR);
  addr = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  if(close(fd) == -1) {
    perror("close");
    exit(1);
  }

  printf("Current string: %*s\n", MEM_SIZE, addr);

  if(argc > 2) {
    if(strlen(argv[2]) >= MEM_SIZE) {
      perror("new-value too large");
      exit(1);
    }

    memset(addr, 0, MEM_SIZE);
    strncpy(addr, argv[2], MEM_SIZE-1);
    // write to file
    if(msync(addr, MEM_SIZE, MS_SYNC) == -1) {
      perror("msync");
    }
    printf("copied %s\n", argv[2]);
  }
}
