#define _DEFAULT_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 64

int main(int argc, char *argv[]) {
  char buf[BUF_SIZE];
  /* int fd = open(argv[1], O_RDONLY); */
  /* int numRead; */
  uid_t uid = getuid();
  uid_t ruid, euid, suid;

  printf("getuid: %d\n", uid);
  getresuid(&ruid, &euid, &suid);
  printf("real: %d, effective: %d, saved set id: %d\n", ruid, euid, suid);
  if(setuid((uid_t)0) == -1) {
    perror("setuid");
    //exit(1);
  }

  getresuid(&ruid, &euid, &suid);
  printf("real: %d, effective: %d, saved set id: %d\n", ruid, euid, suid);

  /* while((numRead = read(fd, buf, BUF_SIZE)) > 0) { */
    /* write(STDOUT_FILENO, buf, numRead); */
  /* } */

  return 0;

}
