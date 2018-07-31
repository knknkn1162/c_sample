#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024

int main(void) {
  int inputFd, outputFd, openFlags;
  mode_t fileParams;
  ssize_t numRead;
  char buf[BUF_SIZE];

  inputFd = open("./Dockerfile", O_RDONLY);
  if(inputFd == -1) {
    perror("open file");
  }

  openFlags = O_CREAT | O_WRONLY | O_TRUNC;
  fileParams = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  outputFd = open("./tmp.txt", openFlags, fileParams);
  if(outputFd == -1) {
    perror("write file");
  }

  while((numRead = read(inputFd, buf, BUF_SIZE)) > 0) {
    if(write(outputFd, buf, numRead) != numRead) {
      perror("couldnt write whole buffer");
      exit(1);
    }
  }

  if(numRead == -1) {
    perror("read");
  }

  if(close(inputFd) == -1) {
    perror("close input");
  }
  if(close(outputFd) == -1) {
    perror("close output");
  }

  exit(EXIT_SUCCESS);
}
