#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/param.h>

void sigchld_ign();
int timerfd();

int main(void) {
  struct epoll_event ev;
  uint64_t read_cnt;
  int efd, tfd, i;
  pid_t pid;

  sigchld_ign();
  efd = eventfd(0, 0);
  tfd = timerfd();
  if((pid = fork()) == 0) {
    int i;
    // create session
    setsid();
    sigchld_ign();


    printf("event: getpid: %d\n", getpid());
    for(i = 0; i < 30; i++) {
      if(fork() == 0) {
        uint64_t write_cnt = (uint64_t)i+1;
        printf("write event will start\n");
        sleep(4);
        printf("Child write to efd: pid: [%d], ppid: [%d]\n", getpid(), getppid());
        write(efd, &write_cnt, sizeof(uint64_t));
        exit(EXIT_SUCCESS);
      }
    }
    wait(NULL);
    printf("Child completed write loop\n");
    exit(EXIT_SUCCESS);
  } else if(pid > 0) { 
    struct epoll_event evs[2];
    int epfd, nfds;
    printf("Parent read to efd, pid:[%d], ppid:[%d]\n", getpid(), getppid());
    epfd = epoll_create(1);
    ev.data.fd = efd;
    ev.events = EPOLLIN;
    // int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    epoll_ctl(epfd, EPOLL_CTL_ADD, efd, &ev);

    ev.data.fd = tfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);

    while(1) {
      // int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
      nfds = epoll_wait(epfd, evs, 2, 50);
      printf("nfds: %d\n", nfds);
      if(nfds == 0) {
        kill(-pid, 0);
        if(errno == ESRCH) {
          printf("end epoll_wait\n");
          break;
        }
        continue;
      }

      for(i = 0; i < nfds; i++) {
        if(evs[i].data.fd == efd) {
          read(efd, &read_cnt, sizeof(uint64_t));
          printf("eventfd Parent read[%ld]\n", read_cnt);
        }
        if(evs[i].data.fd == tfd) {
          read(tfd, &read_cnt, sizeof(uint64_t));
          printf("timefd Parent read[%ld]\n", read_cnt);
        }
      }
    }
  }
  printf("Parent completed read loop\n");
  return 0;
}


void sigchld_ign() {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_NOCLDWAIT;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, 0);
}


int timerfd() {
  //struct timespec cur;
  int fd = timerfd_create(CLOCK_MONOTONIC, 0);
  struct itimerspec val;

  val.it_value.tv_sec = 1;
  val.it_value.tv_nsec = 0;
  val.it_interval.tv_sec = 1;
  val.it_interval.tv_nsec = 0;

  timerfd_settime(fd, 0, &val, 0);
  return fd;
}
