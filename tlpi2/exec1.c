#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t sighupCatch = 0;
volatile sig_atomic_t sigtermCatch = 0;
volatile sig_atomic_t sigcontCatch = 0;
void term_handler(int sig) { sigtermCatch = 1; }
void hup_handler(int sig) { sighupCatch = 1; }
void cont_handler(int sig) { sigcontCatch = 1; }

int main(int argc, char *argv[]) {
  pid_t pid;
  struct sigaction sa;
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = hup_handler;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  sa.sa_handler = term_handler;
  if(sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  sa.sa_handler = cont_handler;
  if(sigaction(SIGCONT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if((pid = fork()) == -1) {
    perror("fork");
    exit(1);

  } else if (pid == 0) {
    // child
    execve(argv[1], &argv[1], (char**)NULL);
    perror("execve");
    exit(1);
  } else {
    wait(NULL);
    printf("wait parent\n");
    exit(EXIT_SUCCESS);
  }

  return 0;
}
