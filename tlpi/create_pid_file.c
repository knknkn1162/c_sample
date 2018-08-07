#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 100
#define CPF_CLOEXEC 1

int createPidFile(const char *progName, const char *pidFile, int flags);

int lockRegion(int fd, int type, int whence, int start, int len);

int main(int argc, char *argv[]) {
  if(createPidFile("mydaemon", "/var/run/mydaemon.pid", 0) == -1) {
    perror("mm");
  }
  return 0;
}


int createPidFile(const char *progName, const char *pidFile, int flags) {
  int fd;
  char buf[BUF_SIZE];

  fd = open(pidFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if(fd == -1) {
    perror("open");
  }

  if(flags & CPF_CLOEXEC) {
    flags = fcntl(fd, F_GETFD);
    flags |= FD_CLOEXEC;
    if(fcntl(fd, F_SETFD, flags) == -1) {
      perror("fcntl");
    }
  }

  if(lockRegion(fd, F_WRLCK, SEEK_SET, 0, 0) == -1) {
    if(errno == EAGAIN || errno == EACCES) {
      perror("lockRegion");
      exit(1);
    }
  }

  snprintf(buf, BUF_SIZE, "%ld\n", (long)getpid());
  printf("buf: %s\n", buf);
  if(write(fd, buf, strlen(buf)) != strlen(buf)) {
    perror("writing pid file");
    exit(1);
  }

  return 0;
}

int lockRegion(int fd, int type, int whence, int start, int len) {
  struct flock fl;
  fl.l_type = type;
  fl.l_whence = whence;
  fl.l_start = start;
  fl.l_len = len;

  return fcntl(fd, F_SETLK, &fl);
}
