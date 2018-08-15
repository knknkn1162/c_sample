#define _GNU_SOURCE
#include <time.h>
#include <utmpx.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  struct utmpx *ut;

  if(argc > 1) {
    if(utmpxname(argv[1]) == -1) {
      perror("utmpxname");
      exit(1);
    }
  }

  setutxent();

  while((ut = getutxent()) != NULL) {
    printf("%s, %d\n", ut->ut_user, ut->ut_pid);
  }

  endutxent();
  return 0;
}
