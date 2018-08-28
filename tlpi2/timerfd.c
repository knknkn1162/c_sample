#include <sys/timerfd.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char *argv[]) {
  struct itimerspec ts;
  struct timespec start, now;
  int fd;
  uint64_t cnt;

  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 50000;
  ts.it_value.tv_sec = 1;
  ts.it_value.tv_nsec = 0;

  fd = timerfd_create(CLOCK_REALTIME, 0);
  if(fd == -1) {
    perror("timerfd_create");
    exit(1);
  }

  // int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
  if(timerfd_settime(fd, 0, &ts, NULL) == -1) {
    perror("timerfd_settime");
  }

  /* if(clock_gettime(CLOCK_MONOTONIC, &start) == -1) { */
    /* perror("clock_gettime"); */
    /* exit(1); */
  /* } */


  cnt = 0;
  while(1) {
    uint64_t exp;
    char buf[100] = {0};
    // exp: number of expirations on the timer
    int num = read(fd, &exp, sizeof(uint64_t));
    if(num != sizeof(uint64_t)) {
      perror("read");
      exit(1);
    }
    memset(buf, '*', exp);
    write(STDOUT_FILENO, buf, exp);
    write(STDOUT_FILENO, "\n", 1);

    cnt += exp;
  }

  return 0;
}
