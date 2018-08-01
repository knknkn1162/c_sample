#include <stdio.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

int main(void) {
  struct tms t;
  clock_t clockTime = clock();
  printf("start clock: %ld\n", clockTime);
  static long clockTicks = 0;
  int i;

  clockTicks = sysconf(_SC_CLK_TCK);
  printf("CLOCKS_PER_SEC=%ld sysconf(_SC_CLK_TCK)=%ld\n\n", (long)CLOCKS_PER_SEC, sysconf(_SC_CLK_TCK));

  printf("clockTicks: %ld\n", clockTicks);
  clockTime = clock();
  if(clockTime == -1) {
    perror("clock");
  }

  printf("%ld, %.4f\n", (long)clockTime, (double)clockTime/CLOCKS_PER_SEC);

  for(i = 0; i < 100000; i++) {
    if(times(&t) == -1) {
      perror("times");
    }

    if(i == 1 || i == 100000-1) {
      printf("%.5f %.5f\n", (double)t.tms_utime/clockTicks, (double)t.tms_stime/clockTicks);
    }
  }
  clockTime = clock() - clockTime;
  printf("%ld clockTime\n", clockTime);

  return 0;

}
