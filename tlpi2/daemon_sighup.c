#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>

static const char *LOG_FILE = "/tmp/ds.log";
static const char *CONFIG_FILE = "/tmp/ds.conf";
static volatile sig_atomic_t hupReceived = 0;
static FILE *logfp;                 /* Log file stream */

#define BD_NO_CHDIR 01
#define BD_NO_CLOSE_FILES 02
#define BD_NO_REOPEN_STD_FDS 04
#define BD_NO_UMASK0 010

#define BD_MAX_CLOSE 8192

static void sighupHandler(int sig) { hupReceived = 1; }
int becomeDaemon(int flags);
static void logMessage(const char *format, ...);
static void logOpen(const char *logFilename);
static void logClose(void);
static void readConfigFile(const char *configFilename);

int main(int argc, char *argv[]) {
  const int SLEEP_TIME = 15;
  int count = 0;
  int unslept;
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = sighupHandler;
  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(becomeDaemon(0) == -1) {
    perror("becomeDaemon");
    exit(1);
  }

  logOpen(LOG_FILE);
  readConfigFile(CONFIG_FILE);

  unslept = SLEEP_TIME;

  while(1) {
    unslept = sleep(unslept);
    if(hupReceived) {
      logClose();
      logOpen(LOG_FILE);
      readConfigFile(CONFIG_FILE);
      hupReceived = 0;
    }

    if(unslept == 0) {
      count++;
      logMessage("Main: %d", count);
      unslept = SLEEP_TIME;
    }
  }

  return 0;
}

/* Write a message to the log file. Handle variable length argument
   lists, with an initial format string (like printf(3), but without
   a trailing newline). Precede each message with a timestamp. */

static void logMessage(const char *format, ...)
{
    va_list argList;
    const char *TIMESTAMP_FMT = "%F %X";        /* = YYYY-MM-DD HH:MM:SS */
#define TS_BUF_SIZE sizeof("YYYY-MM-DD HH:MM:SS")       /* Includes '\0' */
    char timestamp[TS_BUF_SIZE];
    time_t t;
    struct tm *loc;

    t = time(NULL);
    loc = localtime(&t);
    if (loc == NULL ||
           strftime(timestamp, TS_BUF_SIZE, TIMESTAMP_FMT, loc) == 0)
        fprintf(logfp, "???Unknown time????: ");
    else
        fprintf(logfp, "%s: ", timestamp);

    va_start(argList, format);
    vfprintf(logfp, format, argList);
    fprintf(logfp, "\n");
    va_end(argList);
}

/* Open the log file 'logFilename' */

static void logOpen(const char *logFilename)
{
    mode_t m;

    m = umask(077);
    logfp = fopen(logFilename, "a");
    umask(m);

    /* If opening the log fails we can't display a message... */

    if (logfp == NULL)
        exit(EXIT_FAILURE);

    setbuf(logfp, NULL);                    /* Disable stdio buffering */

    logMessage("Opened log file");
}

/* Close the log file */

static void
logClose(void)
{
    logMessage("Closing log file");
    fclose(logfp);
}

/* (Re)initialize from configuration file. In a real application
   we would of course have some daemon initialization parameters in
   this file. In this dummy version, we simply read a single line
   from the file and write it to the log. */
static void readConfigFile(const char *configFilename)
{
    FILE *configfp;
#define SBUF_SIZE 100
    char str[SBUF_SIZE];

    configfp = fopen(configFilename, "r");
    if (configfp != NULL) {                 /* Ignore nonexistent file */
        if (fgets(str, SBUF_SIZE, configfp) == NULL)
            str[0] = '\0';
        else
            str[strlen(str) - 1] = '\0';    /* Strip trailing '\n' */
        logMessage("Read config file: %s", str);
        fclose(configfp);
    }
}

int becomeDaemon(int flags) {
  int maxfd, fd;

  printf("before execution:\n");
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));
  switch(fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      // child continues in the background
      // guaranteed not to be a process group leader
      break;
    default:
      _exit(EXIT_SUCCESS);
  }
  printf("after first fork:\n");
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  if(setsid() == -1) {
    perror("setsid");
    exit(1);
  }

  printf("after setsid\n");
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  // the child is not the session leader and the process can reacquire a controlling terminal.(Because only the session leader can open the controlling terminal.)
  switch(fork()) {
    case -1:
      return -1;
    case 0:
      break;
    default:
      _exit(EXIT_SUCCESS);
  }
  printf("after second fork:\n");
  printf("parent: PID: %ld, PPID: %ld, PGID: %ld, SID=%ld\n", (long)getpid(), (long)getppid(), (long)getpgrp(), (long)getsid(0));

  // clear file mode creation mask to access files
  if(!(flags & BD_NO_UMASK0)) {
    umask(0);
  }

  // Change the process's current working directory, typically to the root directory(/). This is necessary because a daemon usually runs until system shutdown.
  if(!(flags & BD_NO_CHDIR)) {
    chdir("/");
  }

  // Since the daemon has lost its controlling terminal and is running in the background, it makes no sense for the daemon to keep file descriptors 0, 1 and 2 open.
  if(!(flags & BD_NO_CLOSE_FILES)) {
    maxfd = sysconf(_SC_OPEN_MAX);
    if(maxfd == -1) {
      maxfd = BD_MAX_CLOSE;
    }

    for(fd = 0; fd < maxfd; fd++) {
      close(fd);
    }
  }

  // don't perform I/O
  if(!(flags & BD_NO_REOPEN_STD_FDS)) {
    close(STDIN_FILENO);
    fd = open("/dev/null", O_RDWR);

    if(fd != STDIN_FILENO) {
      return -1;
    }
    if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
      return -1;
    }
    if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
      return -1;
    }
  }

  return 0;
}
