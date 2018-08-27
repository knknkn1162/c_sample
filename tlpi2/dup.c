#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

int main(void) {
  int rfd = open("test.txt", O_RDONLY);
  int wfd = open("test.txt", O_WRONLY | O_TRUNC);
  //struct sigaction sa;

  if(rfd == -1) {
    perror("Fd");
    exit(1);
  }
  if(wfd == -1) {
    perror("Fd");
    exit(1);
  }
  /* sa.sa_handler = SIG_IGN; */
  /* sigemptyset(&sa.sa_mask); */
  /* if(sigaction(SIGCHLD, &sa, NULL) == -1) { */
    /* perror("sigaction"); */
    /* exit(1); */
  /* } */

  if(fork() == 0) {
    close(rfd);
    dup2(wfd, STDOUT_FILENO);

    if(close(wfd) == -1) {
      perror("close");
      exit(1);
    }
    execlp("ls", "ls", (char*)NULL);
    exit(1);
  }

  wait(NULL);

  if(fork() == 0) {
    close(wfd);
    dup2(rfd, STDIN_FILENO);
    if(close(rfd) == -1) {
      perror("exit");
    }
    execlp("wc", "wc", "-l", (char*)NULL);
    exit(1);
  }

  /* descriptor need not to be closed */
  wait(NULL);
  exit(1);
}
