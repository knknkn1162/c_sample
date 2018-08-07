#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int fd;
  size_t len;
  char *addr;

  fd = shm_open(argv[1], O_RDWR, 0);
  if(fd == -1) {
    perror("shm_open");
    exit(1);
  }

  len = strlen(argv[2]);
  if(ftruncate(fd, len) == -1) {
    perror("ftruncate");
    exit(1);
  }
  printf("Resized to %ld bytes\n", (long)len);

  addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  if(close(fd) == -1) {
    perror("close");
  }

  printf("copying %ld bytes\n", (long)len);
  memcpy(addr, argv[2], len);

  exit(EXIT_SUCCESS);
}
