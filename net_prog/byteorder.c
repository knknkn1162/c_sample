#include <stdio.h>

int main(int argc, char *argv[]) {
  union {
    short s;
    char c[sizeof(short)];
  } un;

  un.s = 0x1234;
  printf("%d\n", un.c[0] == 0x34);

  return 0;
}
