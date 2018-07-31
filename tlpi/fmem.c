#include <stdio.h>
#include <memory.h>

#define BSZ 48

int main(void) {
  FILE *fp;
  char buf[BSZ];

  memset(buf, 'a', BSZ-2);
  buf[BSZ-2] = '\0';
  buf[BSZ-1] = 'X';

  if((fp = fmemopen(buf, BSZ, "w")) == NULL) {
    perror("fmemopen failed");
  }

  printf("initial buffer contents: %s\n", buf);
  fprintf(fp, "hello world");
  printf("before flush: %s\n", buf);
  fflush(fp);

  printf("after flush: %s\n", buf);
  printf("len of string in buf = %ld\n", (long)strlen(buf));

  fclose(fp);
  return 0;
}
