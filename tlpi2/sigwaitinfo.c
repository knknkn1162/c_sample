#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "req_res.h"

int main(int argc, char *argv[]) {
  int sig;
  siginfo_t si;
  ssize_t num;
  union sigval sv;
  int fd;
  sigset_t mask;
  struct request *req;
  

  while(1) {
    struct response* res = malloc(sizeof(struct response));
    memset(&res, 0, sizeof(struct response));

    sig = sigwaitinfo(&mask, &si);
    if(sig == -1) {
      perror("sigwaitinfo");
      exit(1);
    }
    if(sig == SIGUSR1) {
      req = (struct request*)si.si_value.sival_ptr;
      req->pid = getpid();
      fd = open(req->pathname, O_RDONLY);
      if(fd == -1) {
        perror("open");
        res->mtype = RESP_FAILURE;
        sv.sival_ptr = &res;
        if(sigqueue(req->pid, SIGUSR2, sv) == -1) {
          perror("sigqueue");
          exit(1);
        }
        continue;
      }
      while((num = read(fd, res->buf, BUF_SIZE)) >= 0) {
        if(num > 0) {
          res->mtype = RESP_DATA;
          res->size = num;
          sv.sival_ptr = &res;
        } else {
          res->mtype = RESP_END;
          sv.sival_ptr = &res;
        }
        if(sigqueue(req->pid, SIGUSR2, sv) == -1) { perror("sigqueue");}
      }
    }
  }
}
