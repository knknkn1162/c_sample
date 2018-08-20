#include <stdio.h>
#include <string.h>


int main(void) {
  char* buf = "sdf";
  char buf0[20];

  char *buf00 = memcpy(buf0, buf, 3);
  

  // 0x7fff6038c170 -> 0x7fff6038c170
  printf("%p -> %p\n", buf0, buf00);

  return 0;
}
