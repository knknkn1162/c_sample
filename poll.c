#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/param.h>

void sigchld_ign();
int timerfd();

int main(void) {
  uint64_t read_cnt;
  struct pollfd pollfd[2];
  pid_t pid;

  sigchld_ign();
  int efd = eventfd(0, 0);
  int tfd = timerfd();
  if((pid = fork()) == 0) {
    int i;
    // create session
    setsid();
    sigchld_ign();


    printf("event: getpid: %d\n", getpid());
    for(i = 0; i < 30; i++) {
      if(pid = fork() == 0) {
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
    printf("Parent read to efd pid:[%d], ppid:[%d]\n", getpid(), getppid());
    pollfd[0].fd = efd;
    pollfd[0].events = POLLIN;
    pollfd[1].fd = tfd;
    pollfd[1].events = POLLIN;

    while(1) {
      // int poll(struct pollfd *fds, nfds_t nfds, int timeout);
      int res = poll(pollfd, 2, 50);
      printf("res: %d\n", res);
      if(res == 0) {
        kill(-pid, 0);
        if(errno == ESRCH) { 
          printf("end\n");
          break; 
        }
        continue;
      }
      if(pollfd[0].revents & POLLIN) {
        read(efd, &read_cnt, sizeof(uint64_t));
        printf("eventfd Parent read[%ld]\n", read_cnt);
      }

      if(pollfd[1].revents & POLLIN) {
        read(tfd, &read_cnt, sizeof(uint64_t));
        printf("timerfd Parent read:[%ld]\n", read_cnt);
      }
    }
    printf("Parent completed read loop\n");
  }
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
