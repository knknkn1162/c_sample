#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t gotSignal = 0;
static void handler(int sig) { gotSignal = 1; }
#define TIMER_SIG SIGUSR1


int main(int argc, char *argv[]) {
  struct itimerspec ts;
  struct sigaction sa;
  struct sigevent sev;
  timer_t timer_id;
  
  
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(TIMER_SIG, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // register TIMER_SIG and process with signal.
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = TIMER_SIG;

  // start in the five secs and publish signal in every 1 sec.
  ts.it_value.tv_sec = 5;
  ts.it_value.tv_nsec = 0;
  ts.it_interval.tv_sec = 1;
  ts.it_interval.tv_nsec = 0;

  // int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
  // Note) Link with `-lrt`.
  if(timer_create(CLOCK_REALTIME, &sev, &timer_id) == -1) {
    perror("timer_create");
    exit(1);
  }
  //  int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
  if(timer_settime(timer_id, 0, &ts, NULL) == -1) {
    perror("timer_settime");
    exit(1);
  }

  while(1) {
    if(gotSignal) {
      printf("timer\n");
      gotSignal = 0;
    }
  }
}
