#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>

int main(void) {
  char *buf = crypt("passwd", "qiS4vsD");
  printf("%s\n", buf);
}
