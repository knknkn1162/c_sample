#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  FILE *fp;
  int status;
  char *cmd = "/bin/ls -d * 2> /dev/null";
  char pathName[PATH_MAX];
  int fileCnt = 0;

  fp = popen(cmd, "r");
  if(fp == NULL) {
    perror("popen");
    exit(1);
  }
  
  while(fgets(pathName, PATH_MAX, fp) != NULL) {
    printf("p: %s\n", pathName);
    fileCnt++;
  }

  printf("count: %d\n", fileCnt);
  status = pclose(fp);
  if(status != -1) {
    perror("pclose");
  }
  return 0;
}


