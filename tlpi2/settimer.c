#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t gotSignal = 0;
static void signalHandler(int sig) { gotSignal = 1; }


int main(int argc, char *argv[]) {
  struct sigaction sa;
  struct itimerval itv;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = signalHandler;
  if(sigaction(SIGALRM, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  // Alarm for the first time in 5 secs. alarm every one sec.
  itv.it_value.tv_sec = 5;
  itv.it_value.tv_usec = 0;
  itv.it_interval.tv_sec = 1; // at interval
  itv.it_interval.tv_usec = 0;

  // int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
  if(setitimer(ITIMER_REAL, &itv, NULL) == -1) {
    perror("setitimer");
    exit(1);
  }
  while(1) {
    if(gotSignal) {
      gotSignal = 0;
      printf("got SIGALRM\n");
    }
  }
}
