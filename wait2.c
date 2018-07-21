#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h> // errno variable
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


void signal_handler(int num);

int main(int argc, char** argv) {
  int max_cnt = 100;
  int sec = 4;
  int main_sec = sec*2;
  int i = 0;
  int ret = 0;
  pid_t pid = 0;
  struct sigaction sa;

  sa.sa_handler = signal_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, 0);

  for(i = 0; i < max_cnt; i++) {
    pid = fork();

    if(pid == 0) {
      sleep(sec);
      printf("%d finish\n", i);
      exit(0);
    }
  }

  while(1) {
    if(!sleep(main_sec)) break;
    printf("errno: %d EINTR: %d\n", errno, EINTR);
    if(errno == EINTR) {
      continue;
    }
    break;
  }
  printf("end");
}

// This cannot cope with all SIGCHLD signals at once.(Some may missed)
void signal_handler(int num) {
  pid_t pid;
  int status;

  while(1) {
    // catch zombie
    // return immediately if no child has exited.
    pid = waitpid(-1, &status, WNOHANG);
    // pid = wait( &status );
    if(pid <= 0) {
      return;
    }
    if(WIFEXITED(status)) {
      printf("child %d end\n", pid);
    }
  }
}
