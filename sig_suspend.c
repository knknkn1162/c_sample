#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  sigset_t newmask;

  sigemptyset(&newmask);
  sigaddset(&newmask, SIGQUIT);


  //block
  if(sigsuspend(&newmask) < 0) {
    perror("suspend error");
  }

  return 0;
}
