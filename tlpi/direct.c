#define _GNU_SOURCE // O_DIRECT
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(void) {
  int fd;
  ssize_t numRead;
  size_t length, alignment;
  off_t offset;
  void *buf;

  length = 512*3;
  offset = 0;
  alignment = 512;

  fd = open("/bin/bash", O_RDONLY | O_DIRECT);
  if(fd == -1) {
    perror("open");
    exit(1);
  }
  FILE *fp = fdopen(fd, "r");
  if(fp == NULL) {
    perror("fdopen");
    exit(1);
  }
  printf("fp: %d\n", fp->_fileno);

  buf = (char*)memalign(alignment * 2, length + alignment) + alignment;
  printf("buf: %p\n", buf);

  if(buf == NULL) {
    perror("memalign");
    exit(1);
  }

  if(lseek(fd, offset, SEEK_SET) == -1) {
    perror("lseek");
    exit(1);
  }

  numRead = read(fd, buf, length);
  if(numRead == -1) {
    perror("numRead");
    exit(1);
  }

  printf("Read %ld bytes\n", (long)numRead);

  return 0;
}
