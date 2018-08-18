#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  if(system(argv[1]) == -1) {
    perror("system");
    exit(1);
  }
}


int system(const char *command) {
  sigset_t blockMask, origMask;
  struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
  pid_t childPid;
  int status, savedErrno;

  if(command == NULL) {
    return system(":") == 0;
  }

  sigemptyset(&blockMask);
  sigaddset(&blockMask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &blockMask, &origMask);

  saIgnore.sa_handler = SIG_IGN;
  saIgnore.sa_flags = 0;
  sigemptyset(&saIgnore.sa_mask);
  sigaction(SIGINT, &saIgnore, &saOrigInt);
  sigaction(SIGQUIT, &saIgnore, &saOrigQuit);

  switch(childPid = fork()) {
    case -1:
      status = -1;
      break;
    case 0:
      saDefault.sa_handler = SIG_DFL;
      saDefault.sa_flags = 0;
      sigemptyset(&saDefault.sa_mask);

      // signals with dispositions set to default or ignore are unchanged
      if(saOrigInt.sa_handler != SIG_IGN) {
        sigaction(SIGINT, &saDefault, NULL);
      }

      if(saOrigQuit.sa_handler != SIG_IGN) {
        sigaction(SIGQUIT, &saDefault, NULL);
      }

      sigprocmask(SIG_SETMASK, &origMask, NULL);

      execl("/bin/sh", "sh", "-c", command, (char*)NULL);
      // prevent flushing of any unwritten data in the child's copy of stdio buffers
      _exit(127);
    default:
      while(wait(&status) == -1) {
        if(errno != EINTR) {
          status = -1;
          break;
        }
      }
  }

  savedErrno = errno;
  sigprocmask(SIG_SETMASK, &origMask, NULL);
  sigaction(SIGINT, &saOrigInt, NULL);
  sigaction(SIGQUIT, &saOrigQuit, NULL);


  return status;
}
