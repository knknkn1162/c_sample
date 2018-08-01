#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_LINE 100

int main(void) {
  int fd;
  char line[MAX_LINE];
  ssize_t n;
  char num[] = "50000";


  fd = open("/proc/sys/kernel/pid_max", O_RDWR);
  if(fd == -1) {
    perror("open");
    exit(1);
  }

  n = read(fd, line, MAX_LINE);
  if(n == -1) { 
    perror("read");
    exit(1);
  }

  printf("%.*s", (int)n, line);

  printf("num: %s\n", num);
  if(write(fd, num, strlen(num)) != strlen(num)) {
    perror("write");
    exit(1);
  }

  exit(EXIT_SUCCESS);
}
