#define _XOPEN_SOURCE 600
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char* str2 = "def";
static int handled = 0;

static void signal_handler(int sig) {
  crypt(str2, "xx");
  handled++;
}

int main(void) {
  char* cr1;
  int callNum, mismatch;
  struct sigaction sa;

  cr1 = strdup(crypt("aaa", "xx"));

  if(cr1 == NULL) {
    perror("strdup");
    exit(1);
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = signal_handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }


  for(callNum = 1, mismatch = 0; ; callNum++) {
    if(strcmp(crypt("aaa", "xx"), cr1) != 0) {
      mismatch++;
      printf("Mismatch: %d mismatch: %d, handled:%d\n", callNum, mismatch, handled);
    }
  }

}
