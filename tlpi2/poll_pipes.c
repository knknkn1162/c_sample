#include <time.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  struct pollfd *pollFd;
  int numPipes, j, numWrites;
  int (*pfds)[2];

  numPipes = atoi(argv[1]);

  pfds = calloc(numPipes, sizeof(int[2]));
  if(pfds == NULL) {
    perror("malloc");
    exit(1);
  }

  pollFd = calloc(numPipes, sizeof(struct pollfd));
  if(pollFd == NULL) {
    perror("malloc");
    exit(1);
  }

  for(j = 0; j < numPipes; j++) {
    if(pipe(pfds[j]) == -1) {
      perror("pipe");
    }
  }

  numWrites = atoi(argv[2]);


  for(j = 0; j < numPipes; j++) {
    pollFd[j].fd = pfds[j][0];
    // data canbe read
    pollFd[j].events = POLLIN;
  }

  srandom((int)time(NULL));
  while(1) {
    for(j = 0; j < numWrites; j++) {
      int randPipe;
      randPipe = random() % numPipes;
      printf("Writing to fd: %3d (read fd: %3d)\n", pfds[randPipe][1], pfds[randPipe][0]);
      if(write(pfds[randPipe][1], "a", 1) == -1) {
        perror("write");
      }
    }
    sleep(1);

    if(poll(pollFd, numPipes, -1) == -1) {
      perror("poll");
      exit(1);
    }

    for(j = 0; j < numPipes; j++) {
      // poll system call returns a count of ready file descriptors, and a file descriptor is counted only once.
      if(pollFd[j].revents & POLLIN) {
        char buf;
        printf("Readable: %d %3d\n", j, pollFd[j].fd);
        read(pollFd[j].fd, &buf, 1);
        printf("[%d]read %c\n", j, buf);
      }
    }
  }
}
