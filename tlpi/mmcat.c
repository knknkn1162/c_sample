#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *addr;
  int fd;
  struct stat sb;

  fd = open(argv[1], O_RDONLY);
  if(fstat(fd, &sb) == -1) {
    perror("fstat");
  }

  addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
  }

  if(write(STDOUT_FILENO, addr, sb.st_size) != sb.st_size) {
    perror("partial/failed write");
  }

  exit(EXIT_SUCCESS);
}
