#include <sys/timerfd.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define TV2SEC(tv) ((double)(tv.tv_sec) + (double)(tv.tv_usec / 1000000.0))

int main(void) {
  struct timespec cur;
  struct timeval tv1, tv2;

  int fd = timerfd_create(CLOCK_MONOTONIC, 0);
  clock_gettime(CLOCK_MONOTONIC, &cur);

  struct itimerspec val;
  val.it_value.tv_sec = cur.tv_sec + 2;
  val.it_value.tv_nsec = 0;
  val.it_interval.tv_sec = 0;
  val.it_interval.tv_nsec = 10000000; // 10ms

  timerfd_settime(fd, TFD_TIMER_ABSTIME, &val, 0);
  gettimeofday(&tv1, 0);

  uint64_t read_cnt;
  int cnt;

  for(cnt = 0; cnt < 1000; cnt++) {
    read(fd, &read_cnt, sizeof(uint64_t));
    gettimeofday(&tv2, 0);
    double rtn = TV2SEC(tv2) - TV2SEC(tv1);
    printf("timerfd %d path...[%f]: %ld\n", cnt+1, rtn, read_cnt);
    tv1 = tv2;
  }
  close(fd);
  exit(EXIT_SUCCESS);
}
