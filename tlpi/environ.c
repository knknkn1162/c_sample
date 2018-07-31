#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main(void) {
  char **ep;
  char *buf;

  for(ep = environ; *ep != NULL; ep++) {
    puts(*ep);
  }

  buf = getenv("HOSTNAME");
  printf("** HOSTNAME: %s\n", buf);


  exit(EXIT_SUCCESS);
}
