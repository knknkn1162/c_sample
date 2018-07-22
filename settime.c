#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

void signal_handler(int num);

// gcc settime.c -lrt
int main(void) {
  timer_t timerid;
  struct timespec interval = {1, 0};
  struct timespec initial_expiration = {3, 0};
  // FIrst, display dot in 3 sec. After that increase dots each one sec;
  struct itimerspec val = {interval, initial_expiration};
  struct sigaction sa;

  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGALRM, &sa, NULL) < 0) {
    perror("SIGALRM error");
    exit(-1);
  }

  timer_create(CLOCK_REALTIME, NULL, &timerid);
  timer_settime(timerid, 0, &val, NULL);

  while(1) {
    sleep(100);
    //pause();
  }

  timer_delete(timerid);
  return 0;
}

void signal_handler(int num) {
  char* msg = ".";
  write(1, msg, strlen(msg));
}
