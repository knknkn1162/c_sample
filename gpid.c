#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

void signal_handler(int num);

pid_t g_pid;
int max_cnt;

int main(void) {
  struct sigaction sa;
  int i;
  int status;
  pid_t pid;

  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGHUP, &sa, NULL);

  max_cnt = 15;
  g_pid = 0;
  fprintf(stdout, "parent: [%d][%d]\n", getpid(), getpgrp());

  for(i = 0; i < max_cnt; i++) {
    pid = fork();
    if(pid == 0) {
      fprintf(stdout, "child %d start:\n", i);
      if(g_pid == 0) {
        g_pid = getpid();
      }
      // this pgid of this process is set to g_pid.

      setpgid(0, g_pid);
      fprintf(stdout, "child[%d] %d pgid: %d(%d)\n",getpid(), i, g_pid, getpgrp());
      sleep(3660);
      exit(0);
    } else if(pid > 0) {
      //pid is child process ID
      //fprintf(stdout, "parent process ID: %d, g_pid: %d\n", getpid(), g_pid);
      if(g_pid == 0) { 
        // TODO: Is this OK?
        g_pid = pid;
        sleep(1);
      }
    }
  }

  // stop until the signal has been sent
  pause();
  printf("program end");
  return 0;
}

void signal_handler(int num) {
  pid_t pid;
  int status;
  fprintf(stdout, "-%d\n", g_pid);
  kill(-g_pid, SIGTERM);

  while(max_cnt) {
    //pid = waitpid(-g_pid, &status, WNOHANG);
    pid = wait(&status);
    max_cnt--;
    if(WIFEXITED(status)) {
      fprintf(stdout, "worker %i normally end\n", pid);
    } else {
      fprintf(stdout, "%i status: %d end[%d(%s)]\n", pid, status, errno, strerror(errno));
    }
  }
}
