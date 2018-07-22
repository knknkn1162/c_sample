#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <setjmp.h>

void signal_handler(int num);
void wait_child(int max_cnt);

pid_t g_pid;
int max_cnt;
sigjmp_buf rp;

int main(void) {
  struct sigaction sa;
  int i;

  sigsetjmp(rp, 1);

  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);

  sigaction(SIGHUP, &sa, 0);
  sigaction(SIGINT, &sa, 0);

  max_cnt = 100;
  g_pid = 0;
  fprintf(stdout, "\nparent[%d][%d] start\n", getpid(), getpgrp());

  for(i = 0; i < max_cnt; i++) {
    pid_t pid = fork();

    if(pid == 0) {
      fprintf(stdout, "child[%d][%d],[%d]", i, getpid(), getpgrp());
      if(g_pid == 0) { g_pid = getpid(); }
      setpgid(0, g_pid);
      while(1) { pause(); }
      exit(0);
    } else if (pid > 0) {
      if(g_pid == 0) { g_pid = pid; sleep(1); }
    }
  }

  while(1) { pause(); } return 0;

}


void signal_handler(int num) {
  pid_t pid;
  int status;
  kill(-g_pid, SIGTERM);

  while(max_cnt) {
    pid = wait(&status);

    max_cnt--;
    if(WIFEXITED(status)) {
      fprintf(stdout, "%i %d\n", pid, max_cnt);
    } else {
      fprintf(stdout, "%i, [%d][%s]", pid, errno, strerror(errno));
    }
  }

  if(num == SIGHUP) { 
    fprintf(stdout, "longjmp\n");
    siglongjmp(rp, 1); 
  } else { exit(0); }
}
