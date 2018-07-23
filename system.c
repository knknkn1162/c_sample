#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int system(const char *cmdstr) {
  pid_t pid;
  int status;

  if(cmdstr == NULL) {
    return(1);
  }

  if ((pid = fork()) < 0) {
    status = -1;
  } else if (pid == 0) {
    execl("/bin/sh", "sh", "-c", cmdstr, (char*)0);
    _exit(127);
  } else {
    while(wait(&status) < 0) {
      if (errno != EINTR) { status = -1; break; }
    }
  }

  return status;
}

int main(void) {
  int status;
  if ((status = system("date")) < 0) {
    perror("system error");
  }

  exit(0);
}
