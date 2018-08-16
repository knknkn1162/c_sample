#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

int ttySetCbreak(int fd, struct termios *prevTermios);
static void handler(int sig);
static void tstpHandler(int sig);

static struct termios userTermios;


int main(int argc, char *argv[]) {
  char ch;
  struct sigaction sa, prev;
  ssize_t n;

  if(ttySetCbreak(STDIN_FILENO, &userTermios) == -1) {
    perror("ttySetCbreak");
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handler;
  // get prev sigaction
  if(sigaction(SIGQUIT, NULL, &prev) == -1) {
    perror("sigaction");
    exit(1);
  }
  if(prev.sa_handler != SIG_IGN) {
    if(sigaction(SIGQUIT, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }
  }

  if(sigaction(SIGINT, NULL, &prev) == -1) {
    perror("sigaction");
    exit(1);
  }
  if(prev.sa_handler != SIG_IGN) {
    if(sigaction(SIGINT, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }
  }

  sa.sa_handler = tstpHandler;
  if(sigaction(SIGTSTP, NULL, &prev) == -1) {
    perror("sigaction");
  }
  if(prev.sa_handler != SIG_IGN) {
    if(sigaction(SIGTSTP, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
    }
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

int ttySetCbreak(int fd, struct termios *prevTermios) {
  struct termios t;
  if(tcgetattr(fd, &t) == -1) {
    return -1;
  }

  if(prevTermios != NULL) {
    *prevTermios = t;
  }

  t.c_lflag &= ~(ICANON | ECHO);
  // When  any  of the characters INTR, QUIT, SUSP, or DSUSP are received, generate the corresponding signal
  t.c_lflag |= ISIG;
  // Translate carriage return to newline on input (unless IGNCR is set).
  t.c_iflag &= ~ICRNL;


  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;

  return 0;
}



static void tstpHandler(int sig) {
  struct termios ourTermios; // saved
  sigset_t tstpMask, prevMask;
  struct sigaction sa, prev;
  int savedErrno;

  savedErrno = errno;
  printf("start tstpHandler\n");

  if(tcgetattr(STDIN_FILENO, &ourTermios) == -1) {
    perror("tcgetattr");
    exit(1);
  }

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &userTermios) == -1) {
    perror("tcsetattr");
    exit(1);
  }

  if(sigaction(SIGTSTP, NULL, &prev) == -1) {
    perror("sigaction");
  }
  prev.sa_handler = SIG_DFL;
  if(sigaction(SIGTSTP, &prev, NULL) == -1)  {
    perror("sigaction SIGTSTP..SIG_DFL");
    exit(1);
  }

  raise(SIGTSTP);
  printf("raise sigtstp\n");

  sigemptyset(&tstpMask);
  sigaddset(&tstpMask, SIGTSTP);
  if(sigprocmask(SIG_UNBLOCK, &tstpMask, &prevMask) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  /* execution resumes here after sigcont */
  printf("catch sigcont\n");
  if(sigprocmask(SIG_SETMASK, &prevMask, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = tstpHandler;
  if(sigaction(SIGTSTP, &sa, NULL) == -1) {
    perror("sigaction SIGTSTP");
  }

  if(tcgetattr(STDIN_FILENO, &userTermios) == -1) {
    perror("tcgetattr");
    exit(1);
  }

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &ourTermios) == -1) {
    perror("tcsetattr");
    exit(1);
  }

  errno = savedErrno;
}


static void handler(int sig) {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &userTermios) == -1) {
    perror("tcsetattr");
    exit(1);
  }
  _exit(EXIT_SUCCESS);
}
