#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "msg.h"

#define POPEN_FMT "/bin/cat %s 2> /dev/null"
#define CMD_BUF_SIZE (sizeof(POPEN_FMT) + REQ_MSG_SIZE)

int main(int argc, char *argv[]) {

  int i;
  int num;
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDWAIT;
  sa.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &sa, 0);

  for(i = 1; i < argc; i++) {
    pid_t pid;
    printf("[child] %d start\n", i - 1);

    if((pid = fork()) == -1) {
      perror("fork");
      exit(1);
    } else if (pid == 0) {
      char cmd[REQ_MSG_SIZE];
      FILE *fp;
      char buf[RESP_MSG_SIZE];
      snprintf(cmd, CMD_BUF_SIZE, POPEN_FMT, argv[i]);

      fp = popen(cmd, "r");
      printf("[child(%d)] start `%s`\n", i-1, cmd);
      while(1) {
        if((num = read(fileno(fp), buf, RESP_MSG_SIZE)) < 0) {
          perror("[child] read");
          continue;
        } else if(num == 0) {
          printf("[child(%d)] EOF\n", i-1);
          break;
        } else {
          printf("[child(%d)]> %s\n", i-1, buf);
        }
      }
      pclose(fp);
      printf("[child(%d)] exit\n", i - 1);
      _exit(EXIT_SUCCESS);
    }
  }

  wait(NULL);
  printf("[parent] exit\n");
  return 0;
}
