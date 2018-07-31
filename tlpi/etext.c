#include <stdio.h>

extern void *end, *edata, *etext;

int main(void) {

  printf("end: %p\n", &end);
  printf("edata: %p\n", &edata);
  printf("etext: %p\n", &etext);
}
