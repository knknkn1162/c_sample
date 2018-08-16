#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 100

int main(int argc, char *argv[]) {
  struct termios tp, save;
  char buf[BUF_SIZE];

  if(tcgetattr(STDIN_FILENO, &tp) == -1) {
    perror("tcgetattr");
  }

  save = tp;
  tp.c_lflag &= ~ECHO;
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1) {
    perror("tcsetattr");
  }

  printf("Enter text: ");
  fflush(stdout);
  if(fgets(buf, BUF_SIZE, stdin) == NULL) {
    perror("get end-of-file error on fgets\n");
  } else {
    printf("\nRead: %s\n", buf);
  }

  if(tcsetattr(STDIN_FILENO, TCSANOW, &save) == -1) {
    perror("tcsetattr");
    exit(1);
  }

  exit(EXIT_SUCCESS);
}
