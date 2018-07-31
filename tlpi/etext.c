#include <stdio.h>
#include <unistd.h>

extern char end, edata, etext;

int main(void) {
  void *p = sbrk(0);
  printf("sbrk: %p\n", p);
  printf("end: %p\n", &end);
  printf("sbrk - end: %ld\n", (char*)p - &end);
  printf("edata: %p\n", &edata);
  printf("etext: %p\n", &etext);

}
