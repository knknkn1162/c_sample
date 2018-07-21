#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h> // errno variable


int main(int argc, char** argv) {
  int max_cnt = 100;
  int sec = 4;
  int main_sec = sec*2;
  int i = 0;
  pid_t pid = 0;
  struct sigaction sa;

  // ignore SIGCHLD
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, 0);

  for(i = 0; i < max_cnt; i++) {
    pid = fork();
    // when child process
    if(pid == 0) {
      sleep(sec);
      printf("c");
      exit(0);
    }
  }
  sleep(main_sec);
  printf("\nparent [%d](%s) end\n", errno, strerror(errno));
}
