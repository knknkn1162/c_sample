#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define BUF_SIZE 256

int main(int argc, char *argv[]) {
  int serverFd;
  int dummyFd;
  int flags;
  char *pathName = "/tmp/server";

  if(mkfifo(pathName, O_CREAT | S_IRUSR | S_IWUSR) == -1 && errno != EEXIST) {
    perror("mkfifo");
    exit(1);
  }

  fprintf(stderr, "mkfifo\n");
  serverFd = open(pathName, O_RDONLY);
  if(serverFd == -1) {
    perror("open");
  }

  // open an extra write descriptor, so that we never see EOF
  fprintf(stderr, "open serverFd\n");
  dummyFd = open(pathName, O_WRONLY);
  if(dummyFd == -1) {
    perror("open");
    exit(1);
  }
  
  flags = fcntl(serverFd, F_GETFL);
  if(flags == -1) {
    perror("fcntl");
  }
  if(fcntl(serverFd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl SETFD");
  }

  // echo
  while(1) {
    char buf[BUF_SIZE];
    int numRead;
    write(STDOUT_FILENO, ".", 1);
    // never see EOF
    if((numRead = read(serverFd, buf, BUF_SIZE)) == -1) {
      fprintf(stderr, "errno: %d(%s)\n", errno, strerror(errno));
      continue;
    } else if (numRead == 0) {
      write(STDOUT_FILENO, "0", 1);
    } else {
      write(STDOUT_FILENO, "> ", 2);
      write(STDOUT_FILENO, buf, numRead);
      write(STDOUT_FILENO, "\n", 1);
    }
  }

  return 0;

}
