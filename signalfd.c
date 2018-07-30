#include <sys/signalfd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int signal_fd(void);
void timer_set(void);

int main(void) {
  struct signalfd_siginfo si;
  int fd;
  struct sigevent ev;
  struct itimerspec ts;
  timer_t timer_id;

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  // create new descripter
  fd = signalfd(-1, &mask, 0);


  ev.sigev_notify = SIGEV_SIGNAL;
  ev.sigev_signo = SIGINT;

  ts.it_value.tv_sec = 4;
  ts.it_value.tv_nsec = 0;
  ts.it_interval.tv_sec = 4;
  ts.it_interval.tv_nsec = 0;

  timer_create(CLOCK_MONOTONIC, &ev, &timer_id);
  // CLOCK_REALTIME
  timer_settime(timer_id, 0, &ts, NULL);

  while(1) {
    read(fd, &si, sizeof(struct signalfd_siginfo));
    if(si.ssi_signo == SIGINT) {
      printf("got sigint");
    } else if(si.ssi_signo == SIGQUIT) {
      printf("got sigquit");
    } else {
      printf("unexpected signal");
    }

    if(si.ssi_code == SI_USER) {
      printf("by user");
    } else if (si.ssi_code == SI_KERNEL) {
      printf("by kernel");
    } else if (si.ssi_code = SI_TIMER) {
      printf("by timerup\n");
    } else {
      printf("code: %d\n", si.ssi_code);
    }
  }

  return 0;
}
