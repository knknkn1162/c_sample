#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <string.h>

#define BUF_SIZE 10

int main(int argc, char *argv[]) {
  int nfds;
  fd_set readfds, writefds;
  fd_set readfds_tmp, writefds_tmp;
  int numRead;
  char buf[BUF_SIZE] = {};


  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_SET(STDIN_FILENO, &readfds);
  FD_SET(STDOUT_FILENO, &writefds);
  nfds = MAX(STDIN_FILENO, STDOUT_FILENO) + 1;
  
  while(1) {
    memset(buf, 0, BUF_SIZE);
    memcpy(&readfds_tmp, &readfds, sizeof(fd_set));
    memcpy(&writefds_tmp, &writefds, sizeof(fd_set));
    if(select(nfds, &readfds_tmp, &writefds_tmp, NULL, NULL) == -1) {
      perror("select");
      exit(1);
    }

    if(FD_ISSET(STDIN_FILENO, &readfds_tmp)) {
      fprintf(stderr, "r");
      sleep(1);
      numRead = read(STDIN_FILENO, buf, BUF_SIZE);
    }

    if(FD_ISSET(STDOUT_FILENO, &writefds_tmp)) {
      fprintf(stderr, "w");
      sleep(1);
      write(STDOUT_FILENO, buf, BUF_SIZE);
    }
  }
  return 0;

}
