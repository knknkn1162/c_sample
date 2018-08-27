#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

  if(argc == 1) {
    fprintf(stderr, "usage error\n");
    exit(1);
  }
  // if the server doesn't launch, blocking until the other opens int the O_RDONLY mode.
  int serverFd = open("/tmp/server", O_WRONLY);

  if(write(serverFd, argv[1], strlen(argv[1])) != strlen(argv[1])) {
    perror("write");
    exit(1);
  }
  printf("send message\n");
  return 0;

}
