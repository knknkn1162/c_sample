#include <stdlib.h>
#include <stdio.h>

static void my_exit1(void);

int main(void) {
  if (atexit(my_exit1) != 0) {
    perror("my_exit1 error");
  }
  printf("main is done\n");
  return 0;
}

static void my_exit1(void) {
  printf("my exit handler\n");
}
