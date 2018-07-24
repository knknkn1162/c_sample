#include <errno.h>
#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h>
#include <signal.h>

void sig_hup(int signo) {
  printf("SIGHUP received, pid = %ld\n", (long)getpid());
}

void pr_ids(char *name) {
  printf("%s: pid = %ld, ppid = %ld, pgrp = %ld, tpgrp = %ld\n",
      name, (long)getpid(), (long)getppid(), (long)getpgrp(), (long)tcgetpgrp(STDIN_FILENO));
  fflush(stdout);
}

int main(void) {
  char c;
  pid_t pid;
  struct sigaction sa;

  sa.sa_handler = sig_hup;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);

  if((pid = fork()) < 0) {
    perror("fork error");
  } else if (pid > 0) {
    sleep(5);
  } else {
    pr_ids("child");
    sigaction(SIGHUP, &sa, 0);
    kill(getpid(), SIGTSTP);
    pr_ids("child");
    if (read(STDIN_FILENO, &c, 1) != 1)
      printf("readd error %d on controlling TTY\n", errno);
  }

  return 0;
}
