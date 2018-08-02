#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
  struct stat sb;
  char* fname = "/bin/bzcmp";

  if(lstat(fname, &sb) == -1) {
    perror("lstat");
  }

  switch(sb.st_mode & S_IFMT) {
    case S_IFREG: printf("regular file\n"); break;
    case S_IFLNK: printf("symbolic link\n"); break;
    default: break;
  }

  printf("%s\n", (sb.st_mode & S_ISUID) ? "set-UID ": "");
  printf("%s\n", (sb.st_mode & S_ISGID) ? "set-UID ": "");
  printf("%s\n", (sb.st_mode & S_ISVTX) ? "sticky": "");

  exit(EXIT_SUCCESS);
}

