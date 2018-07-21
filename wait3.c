#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
  int max_cnt = 100;
  int sec = 4;
  int status;
  int i = 0;
  pid_t pid;
  struct sigaction sa;

  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_NOCLDWAIT;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, 0);

  for(i = 0; i < max_cnt; i++) {
    pid = fork();

    if(pid == 0) {
      sleep(sec);
      write(1, "c", 2);
      exit(0);
    }
  }
  // parent
  sleep(sec);
  write(1, "end", 4);
  wait(&status);

  return 0;
}
