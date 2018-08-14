#include <sys/xattr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XATTR_SIZE 10000

int main(int argc, char *argv[]) {
  char list[XATTR_SIZE], value[XATTR_SIZE];
  ssize_t listLen, valueLen;
  int ns, j, k, opt;

  for(j = 1; j < argc; j++) {
    listLen = listxattr(argv[j], list, XATTR_SIZE);
    if(listLen == -1) {
      perror("listxattr");
    }

    for(ns = 0; ns < listLen; ns += strlen(&list[ns]) + 1) {
      printf("name=%s\n", &list[ns]);
      valueLen = getxattr(argv[j], &list[ns], value, XATTR_SIZE);
      printf("value: %.*s\n", (int)valueLen, value);
    }
  }

  exit(EXIT_SUCCESS);
}
