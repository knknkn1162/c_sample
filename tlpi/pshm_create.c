#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  int fd;
  void *addr;
  size_t size = atoi(argv[2]);

  fd = shm_open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

  if(fd == -1) {
    perror("shm_open");
    exit(1);
  }

  if(ftruncate(fd, size) == -1) {
    perror("ftruncate");
    exit(1);
  }

  // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
  addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
  }

  exit(EXIT_SUCCESS);
  shm_unlink(argv[1]);
}
