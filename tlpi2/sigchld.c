#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {write(STDOUT_FILENO, "handler", 8);}

int main(int argc, char *argv[]) {
  struct sigaction sa;
  pid_t pid;
  sa.sa_handler = handler;
  sa.sa_flags = SA_NOCLDSTOP;
  sigemptyset(&sa.sa_mask);

  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  raise(SIGTSTP);

  if((pid = fork()) == -1) {
    perror("fork");
    exit(1);
  } else if (pid == 0)  {
    raise(SIGTSTP);
    sleep(1);
    printf("child exit\n");
    _exit(EXIT_SUCCESS);
  } else {
    //parent
    sleep(7);
    printf("parent");
    exit(EXIT_SUCCESS);
  }


  return 0;
}
