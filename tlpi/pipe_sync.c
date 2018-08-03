#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void errExit(const char* msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char* argv[]) {
  int pfd[2];
  int j, dummy;

  setbuf(stdout, NULL);

  if(pipe(pfd) == -1) {
    errExit("pipe");
  }

  for(j = 1; j < argc; j++) {
    switch(fork()) {
      case -1:
        errExit("fork");
      case 0:
        if(close(pfd[0]) == -1) { errExit("close read for child"); }
        sleep(atoi(argv[j]));
        if(close(pfd[1]) == -1) { errExit("close write for child"); }
        printf("%d PID: %d\n", j, getpid());
        _exit(EXIT_SUCCESS);
      default:
        // parent loops to create next child
        break;
    }
  }

  if(close(pfd[1]) == -1) { errExit("close write for parent"); }

  printf("read");
  // if pfds are all closed, it should be done.
  if(read(pfd[0], &dummy, 1) != 0) { errExit("didnt get eof"); }
  printf("Parent ready to go\n");

  return 0;
}
