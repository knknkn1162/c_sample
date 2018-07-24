#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <asm/ldt.h>   
#include <asm/prctl.h>
#include <sys/prctl.h>


unsigned long get_base(int code) {
  unsigned long base;

  if (arch_prctl(code, &base) != 0) {
    perror("arch_prctl");
  }

  return base;
}


int main() {
  printf("FS base 0x%lx\n", get_base(ARCH_GET_FS));
  printf("GS base 0x%lx\n", get_base(ARCH_GET_GS));

  if (arch_prctl(ARCH_SET_GS, 0x12345678) != 0) {
    perror("ARCH_SET_GS");
  }

  printf("(new) GS base 0x%lx\n", get_base(ARCH_GET_GS));

  printf("Sleeping for 10 seconds, please snapshot me.\n");
  fflush(NULL);
  sleep(10);

  printf("(new) GS base 0x%lx\n", get_base(ARCH_GET_GS));

  return EXIT_SUCCESS;  
}
