#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include "req_res.h"

// static void handler(int sig, siginfo_t *siginfo, void *ucontext)

int main(int argc, char *argv[]) {
  union sigval sv;
  int sig;
  siginfo_t si;
  struct request req;
  sigset_t mask;

  req.pid = getpid();
  strncpy(req.pathname, argv[2], strlen(argv[2]));

  sv.sival_ptr = (void*)&req;
  
  // int sigqueue(pid_t pid, int sig, const union sigval value);
  if(sigqueue(atoi(argv[1]), SIGUSR1, sv)) {
    perror("sigqueue");
    exit(1);
  }

  // response
  while(1) {
    struct response *res;
    // int sigwaitinfo(const sigset_t *set, siginfo_t *info);
    sig = sigwaitinfo(&mask, &si);
    if(sig == -1) {
      perror("sigwaitinfo");
    }
    if(sig != SIGUSR2) {
      continue;
    }
    res = (struct response*)si.si_value.sival_ptr;

    switch(res->mtype) {
      case RESP_FAILURE:
        exit(EXIT_FAILURE);
      case RESP_DATA:
        write(STDOUT_FILENO, res->buf, res->size);
        break;
      case RESP_END:
        printf("end\n");
        exit(EXIT_SUCCESS);
    }
  }
}


