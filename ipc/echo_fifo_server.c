#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define BUF_SIZE 256

int main(int argc, char *argv[]) {
  int serverFd, clientFd;
  int dummyFd;
  struct sigaction sa;


  umask(0);
  if(mkfifo(argv[1], S_IRUSR | S_IWUSR) == -1 && errno != EEXIST) {
    perror("mkfifo");
    exit(1);
  }


  serverFd = open(argv[1], O_RDONLY);
  if(serverFd == -1) {
    perror("open");
  }
  // open an extra write descriptor, so that we never see EOF
  dummyFd = open(argv[1], O_WRONLY);
  if(dummyFd == -1) {
    perror("open");
    exit(1);
  }
 
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if(sigaction(SIGPIPE, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // echo
  while(1) {
    char buf[BUF_SIZE];
    int numRead;
    if((numRead = read(serverFd, buf, BUF_SIZE)) == -1) {
      fprintf(stderr, "errno: %d(%s)\n", errno, strerror(errno));
      continue;
    } else {
      printf(">");
      write(STDOUT_FILENO, buf, numRead);
    }
  }

  return 0;

}
