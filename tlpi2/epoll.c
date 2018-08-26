#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_BUF 10
#define MAX_EVENTS 5

int main(int argc, char *argv[]) {
  int epfd, fd, i, numOpenFds;
  struct epoll_event ev;
  struct epoll_event evlist[MAX_EVENTS];
  char buf[MAX_BUF];

  // create epoll instance
  epfd = epoll_create(argc - 1);
  if(epfd == -1) {
    perror("epoll_create");
    exit(1);
  }
  printf("epoll_create\n");

  // modify the interest list of the epoll instance referred to by the epfd
  for(i = 1; i < argc; i++) {
    fd = open(argv[i], O_RDONLY);
    if(fd == -1) {
      perror("open");
      exit(1);
    }
    printf("Opened %s on fd %d\n", argv[i], fd);

    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      perror("epoll_ctl");
      exit(1);
    }
    printf("epoll_ctl on fd %d\n", fd);
  }

  numOpenFds = argc - 1;

  while(numOpenFds) {
    int ready;
    if((ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1)) == -1) {
      // restart if interrupted by signal
      if(errno == EINTR) {
        continue;
      } else {
        perror("epoll_wait");
        exit(1);
      }
    }

    printf("ready\n");


    for(i = 0; i < ready; i++) {
      if(evlist[i].events & EPOLLIN) {
        printf("epollin\n");
        int num = read(evlist[i].data.fd, buf, MAX_BUF);
        if(num == -1) {
          perror("read");
          exit(1);
        }
        printf("read %d bytes :%s\n", num, buf);
        // if ~EPOLLIN & EPOLLHUP | EPOLLERR
      } else if (evlist[i].events & (EPOLLHUP | EPOLLERR)) {
        printf("closing fd %d\n", evlist[i].data.fd);
        if(close(evlist[i].data.fd) == -1) {
          perror("close");
          exit(1);
        }
        numOpenFds--;
      }
    }
  }
  printf("all file descriptors closed;\n");
  return 0;
}
