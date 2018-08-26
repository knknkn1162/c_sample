#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static volatile sig_atomic_t gotSigio = 0;
static void sigioHandler(int sig) { gotSigio = 1; }

int ttySetCbreak(int fd, struct termios *prevTermios);

int main(int argc, char *argv[]) {

  int flags;
  long cnt;
  struct termios origTermios;
  char ch;
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = sigioHandler;
  if(sigaction(SIGIO, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  sigaction(SIGINT, &sa, NULL);
  // ttySetCbreak: Inappropriate ioctl for device
  /* if(ttySetCbreak(fd, NULL) == -1) { */
    /* perror("ttySetCbreak"); */
  /* } */

  // the process or process group that is to receive signals when I/O is possible on the fd.
  if(fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1) {
    perror("fcntl");
    exit(1);
  }

  // enable I/O possible signalling
  flags = fcntl(STDIN_FILENO, F_GETFL);
  if(fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1) {
    perror("fcntl");
    exit(1);
  }

  if(ttySetCbreak(STDIN_FILENO, &origTermios) == -1) {
    perror("ttySetCbreak");
    exit(1);
  }

  for(cnt = 0; ; cnt++) {
    if(gotSigio) {
      int numRead;
      printf("signal caught\n");
      while(1) {
        if((numRead = read(STDIN_FILENO, &ch, 1)) > 0) {
          printf("cnt=%ld; read %c\n", cnt, ch);
        } else {
          fprintf(stderr, "%d: errno.(%d) %s", numRead, errno, strerror(errno));
          break;
        }
      }
      gotSigio = 0;
    }
  }

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios) == -1) {
    perror("tcsetattr");
    exit(1);
  }


}




int ttySetCbreak(int fd, struct termios *prevTermios) {
  struct termios t;
  if(tcgetattr(fd, &t) == -1) {
    perror("tcgetattr");
  }

  if(prevTermios != NULL) {
    *prevTermios = t;
  }

  t.c_lflag &= ~(ICANON | ECHO);
  t.c_lflag |= ISIG;

  t.c_iflag &= ~ICRNL;

  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;

  if(tcsetattr(fd, TCSAFLUSH, &t) == -1) {
    return -1;
  }

  return 0;
}
