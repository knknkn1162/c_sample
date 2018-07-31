#include <stdio.h>

int main(void) {
  FILE *fp;
  char buf[4096];

  if((fp = fopen("/proc/self/maps", "rw")) == NULL) {
    return -1;
  }

  while(fgets(buf, 4096, fp) != NULL) {
    puts(buf);
  }

  return 0;
}
