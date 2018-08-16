#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

int ttySetRaw(int fd, struct termios *prevTermios);

static struct termios userTermios;

int main(int argc, char *argv[]) {
  char ch;
  struct sigaction sa;
  ssize_t n;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  // save prev termios, userTermios;
  if(ttySetRaw(STDIN_FILENO, &userTermios) == -1) {
    perror("ttySetRaw");
    exit(1);
  }

  setbuf(stdout, NULL);

  while(1) {
    n = read(STDIN_FILENO, &ch, 1);
    if(n == -1) {
      perror("read");
      break;
    }
    if(n == 0) {
      break;
    }

    if(isalpha((unsigned char)ch) ) {
      putchar(tolower((unsigned char)ch));
    } else if (ch == '\n' || ch == '\r') {
      putchar(ch);
    } else if (iscntrl((unsigned char)ch)) {
      printf("^%c", ch^64);
    } else {
      putchar('*');
    }
    if(ch == 'q') {
      break;
    }
  }

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &userTermios) == -1) {
    perror("tcsetattr");
    exit(1);
  }
  exit(EXIT_SUCCESS);

}

int ttySetRaw(int fd, struct termios *prevTermios) {
  struct termios t;
  if(tcgetattr(fd, &t) == -1) {
    return -1;
  }

  if(prevTermios != NULL) {
    *prevTermios = t;
  }

  t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
  t.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR | INPCK | ISTRIP | IXON | PARMRK);

  t.c_oflag &= ~OPOST;
  // take one character
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;

  if(tcsetattr(fd, TCSAFLUSH, &t) == -1) {
    return -1;
  }
  return 0;
}
