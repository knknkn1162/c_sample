#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define BUF_SIZE 256
#define MAX_SNAME 100

int ttySetRaw(int fd, struct termios *prevTermios);
static void ttyReset(void);
static struct termios ttyOrig;

int main(int argc, char *argv[]) {
  int masterFd;
  pid_t childPid;
  char ptsName[MAX_SNAME];
  struct winsize ws;

  // opens an unused pseudoterminal master device, returning a file descriptor that can be used to refer to that device.
  masterFd = posix_openpt(O_RDWR | O_NOCTTY);

  // changes the mode and owner of the slave pseudoterminal device corresponding to the master pseudoterminal referred to by fd.
  grantpt(masterFd);

  unlockpt(masterFd);

  tcgetattr(STDIN_FILENO, &ttyOrig);
  ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);


  if((childPid = fork()) == 0) {
    int slaveFd;
    char buf[BUF_SIZE];
    setsid();

    // returns the name of the slave pseudoterminal device corresponding to the master referred to by fd.
    ptsname_r(masterFd, ptsName, MAX_SNAME);
    close(masterFd);

    slaveFd = open(ptsName, O_RDWR);
    tcsetattr(slaveFd, TCSANOW, &ttyOrig);
    ioctl(slaveFd, TIOCSWINSZ, &ws);

    dup2(slaveFd, STDIN_FILENO);
    dup2(slaveFd, STDOUT_FILENO);
    dup2(slaveFd, STDERR_FILENO);
    // char *shell = getenv("SHELL");
    // 2.d) Call exec to start the terminal oriented program that is to be connected to the psuedoterminal slave.
    // execlp(shell, shell, (char*)NULL);
    while(1) {
      int numRead;
      if((numRead = read(slaveFd, buf, BUF_SIZE)) < 0) {
        perror("numRead");
      }
      fprintf(stderr, "slave stdout\n");

      if(write(slaveFd, buf, numRead) != numRead) {
        perror("write");
        exit(1);
      }
    }
    perror("exec");
    exit(1);
  } else {
    fd_set inFds, tmp_inFds;
    int maxfd;
    char buf[BUF_SIZE];

    atexit(ttyReset);

    FD_ZERO(&inFds);
    FD_SET(STDIN_FILENO, &inFds);
    FD_SET(masterFd, &inFds);
    maxfd = masterFd + 1;

    while(1) {
      tmp_inFds = inFds;
      fprintf(stderr, "before select\n");
      if(select(maxfd, &tmp_inFds, NULL, NULL, NULL) == -1) {
        perror("select");
        continue;
      }

      if(FD_ISSET(STDIN_FILENO, &tmp_inFds)) {
        int numRead;
        if((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) <= 0) {
          exit(EXIT_SUCCESS);
        }
        fprintf(stderr, "master write to masterFd\n");
        if(write(masterFd, buf, numRead) != numRead) {
          perror("write");
          exit(1);
        }
      }

      if(FD_ISSET(masterFd, &tmp_inFds)) {
        int numRead;
        numRead = read(masterFd, buf, BUF_SIZE);
        if(numRead <= 0) {
          exit(EXIT_SUCCESS);
        }
        fprintf(stderr, "master write to STDOUT\n");
        if(write(STDOUT_FILENO, buf, numRead) != numRead) {
          perror("write");
          exit(1);
        }
      }
    }
  }
  return 0;
}


static void ttyReset(void) {
  if(tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1) {
    perror("tcsetattr");
    exit(1);
  }
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
  t.c_iflag &= ~(BRKINT | ICRNL | IGNCR | INLCR | INPCK | ISTRIP | IXON | PARMRK);

  t.c_oflag &= ~OPOST;
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;

  if(tcsetattr(fd, TCSAFLUSH, &t) == -1) {
    return -1;
  }
  return 0;
}

