#include <unistd.h> 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  printf("read uid = %d, euid = %d\n", getuid(), geteuid());
  exit(0);
}
