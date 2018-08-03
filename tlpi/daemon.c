#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BD_NO_CHDIR 01
#define BD_NO_CLOSE_FILES 02
#define BD_NOREOPEN_STD_FDS 04
#define BD_NO_UMASK0 010
#define BD_MAX_CLOSE 8192

int becomeDaemon(int flags) {
  int maxfd, fd;

  printf("PID: %d\n", getpid());
  switch(fork()) {
    case -1: return -1;
    case 0: break;
    default: _exit(EXIT_SUCCESS);
  }

  if(setsid() == -1) {
    perror("setsid");
    return -1;
  }

  printf("%d\n", getpid());

  switch(fork()) {
    case -1: return -1;
    case 0: break;
    default: _exit(EXIT_SUCCESS);
  }
  printf("%d\n", getpid());

  if(!(flags & BD_NO_UMASK0)) {
    umask(0);
  }

  if(!(flags & BD_NO_CHDIR)) {
    chdir("/");
  }

  if(!(flags & BD_NO_CLOSE_FILES)) {
    maxfd = sysconf(_SC_OPEN_MAX);
    if(maxfd == -1) {
      maxfd = BD_MAX_CLOSE;
    }

    for(fd = 0; fd < maxfd; fd++) {
      close(fd);
    }
  }


  if(!(flags & BD_NOREOPEN_STD_FDS)) {
    close(STDIN_FILENO);

    fd = open("/dev/null", O_RDWR);
    if(fd != STDIN_FILENO) { return -1; }
    if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
      return -1;
    }
    if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
      return -1;
    }
  }

  return 0;
}

int main(void) {
  if(becomeDaemon(0) == -1) {
    perror("becomeDaemon");
    exit(1);
  }
  sleep(60);
  return 0;
}
