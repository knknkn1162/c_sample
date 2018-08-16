#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
  struct termios tp;
  int intrChar;

  if(argc == 1) {
    intrChar = fpathconf(STDIN_FILENO, _PC_VDISABLE);
    if(intrChar == -1) {
      perror("VDISABLE");
      exit(1);
    }
  } else if (isdigit((unsigned char) argv[1][0])) {
    intrChar = strtoul(argv[1], NULL, 0);
  } else {
    intrChar = argv[1][0];
  }

  if(tcgetattr(STDIN_FILENO, &tp) == -1) {
    perror("tcgetattr");
  }
  tp.c_cc[VINTR] = intrChar;
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1) {
    perror("tcsetattr");
  }

  return 0;
}
