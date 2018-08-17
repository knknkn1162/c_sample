#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  int i = 0;
  while(1) {
    printf("stdout, i = %d\n", i++);
    fflush(stdout);
    sleep(1);
  }
  return 0;
}
