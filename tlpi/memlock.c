#define _DEFAULT_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void displayMincore(char* addr, size_t length);

int main(int argc, char *argv[]) {
  char *addr;
  size_t len, lockLen;
  long pageSize, stepSize, j;

  pageSize = sysconf(_SC_PAGESIZE);
  if(pageSize == -1) {
    perror("_SC_PAGESIZE");
  }

  len = atoi(argv[1]) * pageSize;
  stepSize = atoi(argv[2]) * pageSize;
  lockLen = atoi(argv[3]) * pageSize;

  addr = mmap(NULL, len, PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if(addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  printf("allocated %ld (%#lx) bytes starting at %p\n", (long)len, (unsigned long)len, addr);
  printf("before mlock\n");

  displayMincore(addr, len);

  for(j = 0; j + lockLen <= len; j+= stepSize) {
    if(mlock(addr + j, lockLen) == -1) {
      perror("mlock");
      exit(1);
    }
  }

  printf("After lock\n");
  displayMincore(addr, len);

  exit(EXIT_SUCCESS);
}

static void displayMincore(char *addr, size_t length) {
  unsigned char *vec;
  long pageSize, numPages, j;

  pageSize = sysconf(_SC_PAGESIZE);
  numPages = (length + pageSize - 1) / pageSize;
  vec = malloc(numPages);

  if(vec == NULL) {
    perror("malloc");
    exit(1);
  }

  if(mincore(addr, length, vec) == -1) {
    perror("mincore");
    exit(1);
  }

  for(j = 0; j < numPages; j++) {
    printf("%c", (vec[j] & 1) ? '*' : '.');
  }

  free(vec);
}
