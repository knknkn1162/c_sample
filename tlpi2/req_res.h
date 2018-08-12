#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024

struct request {
  pid_t pid;
  char pathname[PATH_MAX];
};

struct response {
  int mtype;
  size_t size;
  char buf[BUF_SIZE];
};

#define RESP_FAILURE 0x01
#define RESP_DATA 0x02
#define RESP_END 0x04
