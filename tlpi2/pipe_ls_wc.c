#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[]) {
  int pfd[2];
  struct sigaction sa;

  sa.sa_flags = SA_NOCLDWAIT;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  if(fork() == 0) {
    close(pfd[0]);
    dup2(pfd[1], STDOUT_FILENO);
    close(pfd[1]);
    execlp("ls", "ls", (char*)NULL);
    exit(1);
  }

  // generate next process
  if(fork() == 0) {
    // never ends witout closing the output file descriptor
    close(pfd[1]);
    dup2(pfd[0], STDIN_FILENO);
    close(pfd[0]);
    execlp("wc", "c", "-l", (char*)NULL);
    exit(1);
  }
  close(pfd[0]);
  // never ends witout closing the output file descriptor
  close(pfd[1]);

  wait(NULL);
  return 0;
}
