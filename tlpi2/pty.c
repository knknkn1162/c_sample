git#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define MAX_SNAME 100
#define BUF_SIZE 256

int ptyMasterOpen(char *slaveName, size_t snLen);
pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen, const struct termios *slaveTermios, const struct winsize *slaveWS);
int ttySetRaw(int fd, struct termios *prevTermios);
static struct termios ttyOrig;
static void ttyReset(void);

int main(int argc, char *argv[]) {
  pid_t childPid;
  struct winsize ws;
  char slaveName[MAX_SNAME];
  int masterFd, slaveFd;
  char *shell;

  if(tcgetattr(STDIN_FILENO, &ttyOrig) == -1) {
    perror("tcgetattr");
    exit(1);
  }
  if(ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0) {
    perror("ioctl TIOCGIWINSZ");
    exit(1);
  }

  if((childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws)) == -1) {
    perror("ptyfork");
    exit(1);
  } else if (childPid == 0) {
    // child
    shell = getenv("SHELL");
    if(shell == NULL || *shell == '\0') {
      shell = "bin/sh";
    }
    printf("[child] execlp start\n");
    execlp(shell, shell, (char*)NULL);
    // never reaches!!
    perror("execlp");
  } else {
    fd_set inFds;
    char buf[BUF_SIZE];
    int numRead;
    // parent
    slaveFd = open("typescript", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    if(slaveFd == -1) {
      perror("open typescript");
      exit(1);
    }

    ttySetRaw(STDIN_FILENO, &ttyOrig);
    if(atexit(ttyReset) != 0) {
      perror("atexit");
    }

    while(1) {
      // wait on select(1)
      FD_ZERO(&inFds);
      FD_SET(STDIN_FILENO, &inFds);
      FD_SET(masterFd, &inFds);

      // int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
      if(select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1) {
        perror("select");
      }

      if(FD_ISSET(STDIN_FILENO, &inFds)) {
        numRead = read(STDIN_FILENO, buf, BUF_SIZE);
        if(numRead <= 0) {
          exit(EXIT_SUCCESS);
        }
        if(write(masterFd, buf, numRead) != numRead) {
          perror("write");
          exit(1);
        }
      }

      if(FD_ISSET(masterFd, &inFds)) {
        numRead = read(masterFd, buf, BUF_SIZE);
        if(numRead <= 0) {
          exit(EXIT_SUCCESS);
        }
        if(write(STDOUT_FILENO, buf, numRead) != numRead) {
          perror("write");
          exit(1);
        }
        if(write(slaveFd, buf, numRead) != numRead) {
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


// create child process that is connected to the parent by a pseudoterminal pair;
pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen, const struct termios *slaveTermios, const struct winsize *slaveWS) {
  int mfd, slaveFd, savedErrno;
  pid_t childPid;
  char slname[MAX_SNAME];

  mfd = ptyMasterOpen(slname, MAX_SNAME);
  if(mfd == -1) {
    return -1;
  }

  if(slaveName != NULL) {
    if(strlen(slname) < snLen) {
      strncpy(slaveName, slname, snLen);

    } else {
      close(mfd);
      errno = EOVERFLOW;
      return -1;
    }
  }

  childPid = fork();
  if(childPid == -1) {
    savedErrno = errno;
    close(mfd);
    errno = savedErrno;
    return -1;
  }

  if(childPid > 0) {
    *masterFd = mfd;
    return childPid;
  }

  // start a new session
  if(setsid() == -1) {
    perror("setsid");
    exit(1);
  }

  // no longer need master file descriptor
  close(mfd);

  slaveFd = open(slname, O_RDWR);
  if(slaveFd == -1) {
    perror("slave");
  }

  if(ioctl(slaveFd, TIOCSCTTY, 0) == -1) {
    perror("ioctl");
    exit(1);
  }

  if(slaveTermios != NULL) {
    if(tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1) {
      perror("ptyFork");
      exit(1);
    }
  }

  if(slaveWS != NULL) {
    if(ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1) {
      perror("ptyfork winsz");
      exit(1);
    }
  }

  // to run on a conventional terminal
  if(dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO) {
    perror("dup2");
    exit(1);
  }
  if(dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO) {
    perror("dup2");
    exit(1);
  }
  if(dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO) {
    perror("dup2");
    exit(1);
  }

  if(slaveFd > STDERR_FILENO) {
    close(slaveFd);
  }

  return 0;
}


// return masterFd
int ptyMasterOpen(char *slaveName, size_t snLen) {
  int masterFd, savedErrno;
  char *p;

  // open the psuedo terminal master
  masterFd = posix_openpt(O_RDWR | O_NOCTTY);
  if(masterFd == -1) {
    return -1;
  }

  // creates a child process that executes a set-user-ID-root program.
  if(grantpt(masterFd) == -1) {
    savedErrno = errno;
    close(masterFd);
    errno = savedErrno;
    return -1;
  }

  // to allow the calling process to perform whatever initialization is required for the pseudoterminal slave
  if(unlockpt(masterFd) == -1) {
    savedErrno = errno;
    close(masterFd);
    return -1;
  }

  p = ptsname(masterFd);
  if(p == NULL) {
    savedErrno = errno;
    close(masterFd);
    errno = savedErrno;
    return -1;
  }

  printf("master pts name: %s\n", ptsname(masterFd));

  strncpy(slaveName, p, snLen);

  return masterFd;
}
