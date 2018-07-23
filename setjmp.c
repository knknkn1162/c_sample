#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>


static void f1(int);
static void f2(void);

static jmp_buf jmpbuffer;

int main(void) {
  int val = 1;
  if (setjmp(jmpbuffer) != 0) {
    // if you add compiler optimization, this is 1..
    printf("after setjmp: %d\n", val);
    exit(0);
  }
  val = 100;
  f1(val);

  exit(0);
}

static void f1(int i) {
  printf("%d\n", i);
  f2();
}

static void f2(void) {
  longjmp(jmpbuffer, 1);
}
