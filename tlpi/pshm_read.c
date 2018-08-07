#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int fd;
  char *addr;
  struct stat sb;

  fd = shm_open(argv[1], O_RDONLY, 0);
  if(fd == -1) {
    perror("shm_open");
    exit(1);
  }

  if(fstat(fd, &sb) == -1) {
    perror("fstat");
    exit(1);
  }

  addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
  }
  if(close(fd) == -1) {
    perror("close");
    exit(1);
  }

  write(STDOUT_FILENO, addr, sb.st_size);
  printf("\n");
  exit(EXIT_SUCCESS);
}
