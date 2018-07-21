#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void handler(int signo, siginfo_t* siginfo, void* dummy);

int main() {
  char* p = NULL;
  struct sigaction sa;
  int pagesize = getpagesize();

  sa.sa_sigaction = handler;
  sa.sa_flags = SA_SIGINFO;

  sigemptyset(&sa.sa_mask);
  sigaction(SIGSEGV, &sa, 0);

  p = malloc(1024 * pagesize - 1);
  p = (char*)((long)(p + pagesize - 1) & ~(pagesize - 1));
  mprotect(p, 1024, PROT_READ);

  p[0x123] = 'a';

  return 0;
}

void handler(int signo, siginfo_t* siginfo, void* dummy) {
  fprintf(stdout, "segmentation fault!!\n");
  exit(1);
}
