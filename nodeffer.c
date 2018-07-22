#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

void signal_handler(int no);

int main() {
  char buf[256];
  int ret;
  struct sigaction sa;

  sa.sa_handler = signal_handler;
  sa.sa_flags = SA_RESTART | SA_NODEFER;

  if(sigaction(SIGINT, &sa, NULL) != 0) {
    fprintf(stderr, "sigaction error\n");
    exit(1);
  }

  while(1) {
    memset(buf, '\0', sizeof(buf));
    printf("read wait\n");
    ret = read(0, buf, sizeof(buf));

    if(ret <= 0) {
      fprintf(stderr, "read error![%d][%s]\n", errno, strerror(errno));
      exit(1);
    }
    write(1, buf, ret);
  }
  
  return 0;
}

void signal_handler(int no) {
  char* mes1 = "signal get\n";
  char* mes2 = "signal end\n";

  write(1, mes1, strlen(mes1));
  sleep(10);
  write(1, mes2, strlen(mes2));
}
