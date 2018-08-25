#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
  char *addr;
  int fd;
  struct stat sb;

  fd = open(argv[1], O_RDONLY);
  if(fd == -1) {
    perror("open");
    exit(1);
  }

  if(fstat(fd, &sb) == -1) {
    perror("fstat");
    exit(1);
  }

  // private file mapping
  addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  // no longer need
  if(close(fd) == -1) {
    perror("close");
    exit(1);
  }

  if(write(STDOUT_FILENO, addr, sb.st_size) != sb.st_size) {
    perror("write");
    exit(1);
  }

  if(munmap(addr, sb.st_size) == -1) {
    perror("munmap");
    exit(1);
  }

  return 0;


}
