#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>


#define BD_NO_CHDIR 01
#define BD_NO_CLOSE_FILES 02
#define BD_NO_REOPEN_STD_FDS 04
#define BD_NO_UMASK0 010

#define BD_MAX_CLOSE 8192

int becomeDaemon(int flags);

int main(int argc, char *argv[]) {
  if(becomeDaemon(0) == -1) {
    perror("becomeDaemon");
    exit(1);
  }

  return 0;
}

int becomeDaemon(int flags) {
  int maxfd, fd;

  printf("before execution:\n");
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      // child continues in the background
      // guaranteed not to be a process group leader
      break;
    default:
      printf("[parent] PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
      _exit(EXIT_SUCCESS);
  }

  printf("after first fork:\n");
  printf("[orphaned] PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  // if the calling process is a process group leader, setsid fails with the err EPERM.
  if(setsid() == -1) {
    perror("setsid");
    exit(1);
  }

  printf("after setsid\n");
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
  printf("tcgetsid %d\n", tcgetsid(STDIN_FILENO));

  // the child is not the session leader and the process can reacquire a controlling terminal.(Because only the session leader can open the controlling terminal.)
  switch(fork()) {
    case -1:
      return -1;
    case 0:
      break;
    default:
      _exit(EXIT_SUCCESS);
  }
  printf("after second fork:\n");
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  // clear file mode creation mask to access files
  if(!(flags & BD_NO_UMASK0)) {
    umask(0);
  }

  // Change the process's current working directory, typically to the root directory(/). This is necessary because a daemon usually runs until system shutdown.
  if(!(flags & BD_NO_CHDIR)) {
    chdir("/");
  }

  // Since the daemon has lost its controlling terminal and is running in the background, it makes no sense for the daemon to keep file descriptors 0, 1 and 2 open.
  if(!(flags & BD_NO_CLOSE_FILES)) {
    maxfd = sysconf(_SC_OPEN_MAX);
    if(maxfd == -1) {
      maxfd = BD_MAX_CLOSE;
    }

    for(fd = 0; fd < maxfd; fd++) {
      close(fd);
    }
  }

  // don't perform I/O
  if(!(flags & BD_NO_REOPEN_STD_FDS)) {
    close(STDIN_FILENO);
    fd = open("/dev/null", O_RDWR);

    if(fd != STDIN_FILENO) {
      return -1;
    }
    if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
      return -1;
    }
    if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
      return -1;
    }
  }

  return 0;
}
