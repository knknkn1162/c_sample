#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>


static volatile sig_atomic_t sigCatch = 0;
void handler(int sig) { sigCatch = 1; }

int main(int argc, char *argv[]) {
  int pfd[2];
  int num;
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGPIPE, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }


  pipe(pfd);


  close(pfd[0]);

  while(1) {
    if((num = write(pfd[1], "987", 4)) == -1) {
      if(errno == EPIPE) {
        fprintf(stdout, "sigpipe\n");
        break;
      }
      perror("write");
      exit(1);
    } else {
      fprintf(stdout, "> %d\n", num);
    }
  }

  printf("sss\n");

  return 0;
}
