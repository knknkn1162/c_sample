#define _GNU_SOURCE
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <syslog.h>

#define BUF_SIZE 256
#define MAX_SNAME 100

int ttySetRaw(int fd, struct termios *prevTermios);
static struct termios ttyOrig;
static void ttyReset(void);

int main(void) {
  int masterFd;
  pid_t childPid;
  char ptsName[MAX_SNAME];
  struct winsize ws;

  // 1. open the psuedoterminal master device
  masterFd = posix_openpt(O_RDWR | O_NOCTTY);

  // creates a child process that executes a set-user-ID-root program.
  grantpt(masterFd);

  // allow the calling process to perform whatever initialization is required for the pseudoterminal slave
  unlockpt(masterFd);


  // retrieve pts name, typically "/dev/pts/* (UNIX 98 master clone device)"
  ptsname_r(masterFd, ptsName, MAX_SNAME);

  // save termios settings
  tcgetattr(STDIN_FILENO, &ttyOrig);
  // save window size settings.
  ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

  // 2. create child process that is connected to the parent by a pseudoterminal pair;
  // The Child process operates slaveFd
  if((childPid = fork()) == 0) {
    char buf[BUF_SIZE];
    int slaveFd;
    // 2.a) Call setsid to start a new session, of which the child is the session leader. This step also causes the child to lose its controlling terminal.
    setsid();

    // no longer need master file descriptor
    close(masterFd);

    // 2.b) Open the psuedoterminal slave device that corresponds to the master device. Since the child process is a session leader, and it doesn't have a controlling terminal, the psuedo terminal slave becomes the controlling terminal for the child process.
    // Make the given terminal the controlling terminal of the calling process.  The calling process must be a session  leader and not have a controlling terminal already
    slaveFd = open(ptsName, O_RDWR);
    // This code allows our ptyFork() function to work on BSD platforms, where a controlling terminal can be acquired only as a consequence of an explicit TIOCSCTTY operation
    ioctl(slaveFd, TIOCSCTTY, 0);

    // reflect termios settings on the slaveFd
    tcsetattr(slaveFd, TCSANOW, &ttyOrig);

    // reflect window size settings on the slaveFd
    ioctl(slaveFd, TIOCSWINSZ, &ws);

    // 2.c) Use dup to duplicate the file descriptor for the slave device on standard input, output and error
    dup2(slaveFd, STDIN_FILENO);
    dup2(slaveFd, STDOUT_FILENO);
    dup2(slaveFd, STDERR_FILENO);

    // never reaches!!
    while(1) {
      int numRead;
      char resp[BUF_SIZE] = {};
      if((numRead = read(slaveFd, buf, BUF_SIZE)) <= 0) {
        perror("numRead");
        exit(1);
      }

      snprintf(resp, BUF_SIZE, "> %s", buf);

      syslog(LOG_INFO, "[slave] write: %.*s (%d)", numRead+2, resp, numRead+2);
      if(write(slaveFd, resp, numRead + 2) != numRead + 2) {
        perror("write");
        exit(1);
      }
    }
    perror("execlp");
    exit(1);

  } else {
    // parent operates masterFd.
    fd_set inFds;
    char buf[BUF_SIZE];
    int numRead;

    // Place the terminal in the raw mode, so that all input characters are passed directly to the script program without begin modified by the terminal driver.
    ttySetRaw(STDIN_FILENO, &ttyOrig);
    atexit(ttyReset);

    while(1) {
      syslog(LOG_INFO, "[master] start select loop");
      memset(buf, 0, BUF_SIZE);
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
        syslog(LOG_INFO, "[master] STDIN_FILENO -> masterFd: %.*s (%d)", numRead, buf, numRead);
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
        syslog(LOG_INFO, "[master] masterFd -> STDOUT_FILENO: %.*s (%d)", numRead, buf, numRead);
        if(write(STDOUT_FILENO, buf, numRead) != numRead) {
          perror("write");
          exit(1);
        }
      }
    }
  } // end fork
  return 0;
}

// restore the tty settings to the state at the startup
static void ttyReset(void) {

  if(tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1) {
    perror("tcsetattr");
    exit(1);
  }
}

// Handle in raw mode which is one of the noncannonical modes.
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
  //take one character
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;

  if(tcsetattr(fd, TCSAFLUSH, &t) == -1) {
    return -1;
  }
  return 0;
}
