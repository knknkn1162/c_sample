#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define BUF_SIZE 100

static void handler(int sig) { printf("sigttin\n"); }

int main(int argc, char* argv[]) {
  char buf[BUF_SIZE];
  int numRead;
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGTTIN, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) >= 0) {
    if(errno == EIO) { perror("xx"); exit(1); }
    buf[numRead] = '\0';
    printf("%s\n", buf);
  }
  return 0;
}
