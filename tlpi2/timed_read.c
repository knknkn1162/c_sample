#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 100

static void handler(int sig) {
  write(STDOUT_FILENO, "caught signal\n", 15);
}

int main(void) {
  struct sigaction sa;
  char buf[BUF_SIZE];
  ssize_t numRead;
  int savedErrno;

  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  sa.sa_handler = handler;
  if(sigaction(SIGALRM, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  alarm(5);
  printf("alarm start\n");

  numRead = read(STDIN_FILENO, buf, BUF_SIZE-1);

  savedErrno = errno;
  alarm(0);
  errno = savedErrno;

  if(numRead == -1) {
    if(errno == EINTR) {
      perror("read timed out\n");
      exit(1);
    } else {
      printf("read");
    }
  } else {
    printf("%s \n", buf);
  }
    exit(EXIT_SUCCESS);
}
