#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>

#define BD_NO_CHDIR 01
#define BD_NO_CLOSE_FILES 02
#define BD_NOREOPEN_STD_FDS 04
#define BD_NO_UMASK0 010
#define BD_MAX_CLOSE 8192

int becomeDaemon(int flags);
static void logMessage(const char *format, ...);
static void logOpen(const char *logFileName);
static void logClose(void);
static void sighupHandler(int sig);
static void readConfigFile(const char *configFileName);

static FILE *logfp;
static const char *LOG_FILE = "/tmp/ds.log";
static const char *CONFIG_FILE = "/tmp/ds.conf";

static volatile sig_atomic_t hupReceived = 0;

int main(void) {
  const int SLEEP_TIME = 15;
  int count = 0;
  int unslept;
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = sighupHandler;

  if(sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sighup sigaction");
    exit(1);
  }

  logOpen(LOG_FILE);
  readConfigFile(CONFIG_FILE);

  unslept= SLEEP_TIME;

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
      logMessage("Main: %d\n", count);
      unslept = SLEEP_TIME;
    }
  }
  return 0;
}

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

static void readConfigFile(const char *configFilename) {
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

static void logClose(void)
{
    logMessage("Closing log file");
    fclose(logfp);
}

static void sighupHandler(int sig) { hupReceived = 1; }

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


int becomeDaemon(int flags) {
  int maxfd, fd;

  printf("PID: %d\n", getpid());
  switch(fork()) {
    case -1: return -1;
    case 0: break;
    default: _exit(EXIT_SUCCESS);
  }

  if(setsid() == -1) {
    return -1;
  }

  printf("%d\n", getpid());

  switch(fork()) {
    case -1: return -1;
    case 0: break;
    default: _exit(EXIT_SUCCESS);
  }
  printf("%d\n", getpid());

  if(!(flags & BD_NO_UMASK0)) {
    umask(0);
  }

  if(!(flags & BD_NO_CHDIR)) {
    chdir("/");
  }

  if(!(flags & BD_NO_CLOSE_FILES)) {
    maxfd = sysconf(_SC_OPEN_MAX);
    if(maxfd == -1) {
      maxfd = BD_MAX_CLOSE;
    }

    for(fd = 0; fd < maxfd; fd++) {
      close(fd);
    }
  }


  if(!(flags & BD_NOREOPEN_STD_FDS)) {
    close(STDIN_FILENO);

    fd = open("/dev/null", O_RDWR);
    if(fd != STDIN_FILENO) { return -1; }
    if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
      return -1;
    }
    if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
      return -1;
    }
  }

  return 0;
}
