#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handler(int sig) {}

int main(int argc, char *argv[]) {
  struct winsize ws;
  struct sigaction sa;

  printf("%s %s\n", ttyname(STDIN_FILENO), ttyname(STDOUT_FILENO));
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if(sigaction(SIGWINCH, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    pause();

    if(ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) {
      perror("ioctl");
    }
    printf("Caught SIGWINCH, new window size:  %d * %d\n", ws.ws_row, ws.ws_col);
  }

  return 0;
}

