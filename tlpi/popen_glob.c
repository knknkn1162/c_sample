#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h> // PATH_MAX

#define POPEN_FMT "/bin/ls %s 2> /dev/null"
#define PAT_SIZE 50
#define PCMD_BUF_SIZE (sizeof(POPEN_FMT) + PAT_SIZE)

int main(void) {
  char pat[PAT_SIZE];
  char popenCmd[PCMD_BUF_SIZE];
  FILE *fp;
  int badPattern;
  int len, status, fileCnt, j;
  char pathName[PATH_MAX];

  while(1) {
    printf("pattern: ");
    fflush(stdout);
    if(fgets(pat, PAT_SIZE, stdin) == NULL) {
      break;
    }
    len = strlen(pat);
    if(len <= 1) continue;
    if(pat[len-1] == '\n') {
      pat[len-1] = '\0';
    }

    for(j = 0, badPattern = 0; j < len && !badPattern; j++) {
      // ctype.h
      if(!isalnum((unsigned char)pat[j]) && strchr("_*?[^-.]", pat[j]) == NULL) {
        badPattern = 1;
      }
    }
    if(badPattern) {
      printf("Bad pattern character: %c\n", pat[j-1]);
      continue;
    }

    snprintf(popenCmd, PCMD_BUF_SIZE, POPEN_FMT, pat);
    popenCmd[PCMD_BUF_SIZE - 1] = '\0';

    printf("* cmd: %s\n", popenCmd);
    fp = popen(popenCmd, "r");
    if(fp == NULL) {
      printf("popen failed\n");
      continue;
    }

    fileCnt = 0;
    while(fgets(pathName, PATH_MAX, fp) != NULL) {
      printf("+ %s", pathName);
      fflush(stdout);
      fileCnt++;
    }


    status = pclose(fp);
    printf("%d matching file\n", fileCnt);
    printf("pclose() status == %#x\n", (unsigned int)status);
  }

  exit(EXIT_SUCCESS);
}
