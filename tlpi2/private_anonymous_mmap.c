#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 4096


int main(int argc, char *argv[]) {
  char *addr;
  int numRead;

  // private anonymous mapping
  addr = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  if((numRead = read(STDIN_FILENO, addr, BUF_SIZE)) < 0) {
    perror("write");
    exit(1);
  }

  if(write(STDOUT_FILENO, addr, numRead) == -1) {
    perror("write");
    exit(1);
  }

  if(munmap(addr, BUF_SIZE) == -1) {
    perror("munmap");
    exit(1);
  }

  return 0;


}
