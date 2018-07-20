#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>

int STDOUT = 1;
/* output
 * ....Alarm clock
 */
int main(void) {
  alarm(4);

  while(1) {
    write(STDOUT, ".", 2);
    sleep(1);
  }
  return 0;
}
