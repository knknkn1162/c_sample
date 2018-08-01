#define _GNU_SOURCE
#include <sys/utsname.h>
#include <stdio.h>


int main(void) {
  struct utsname uts;
  if(uname(&uts) == -1) {
    perror("uname");
  }

  printf("%s, %s, %s, %s, %s\n", uts.nodename, uts.sysname, uts.release, uts.version, uts.machine);
  printf("%s\n", uts.domainname);

  return 0;
}
