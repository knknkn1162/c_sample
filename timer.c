#include <stdlib.h>
#include <string.h>
#include <unistd.h> // write, sleep
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

void sig_handler(int sig_no);

int while_count = 0;
int limit_loop = 0;
int loop_count = 0;

int main(void) {
  struct timeval val = {1, 0};
  struct itimerval timerval = {val, val};
  struct sigaction sa;

  setitimer(ITIMER_REAL, &timerval, NULL);

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = sig_handler;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGALRM, &sa, NULL);


  limit_loop = 4;
  while(limit_loop != loop_count) { while_count++; }

  //filled with zeros
  timerclear(&timerval.it_interval);
  timerclear(&timerval.it_value);
  setitimer(ITIMER_REAL, &timerval, 0);
  return 0;

  // int timer_settime(timer_t timerid, int flags,
  //                       const struct itimerspec *new_value,
  //                       struct itimerspec *old_value);

}

void sig_handler( int sig_no ) {
    static int sec = 0;
    printf( "%2d sec passed! %d\n", sec, while_count );
    sec++;
    while_count = 0;
    loop_count ++;
}
