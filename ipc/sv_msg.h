#include <limits.h>

#define REQ_MSG_SIZE PATH_MAX
#define RESP_MSG_SIZE 256

struct request {
  long mtype;
  char mtext[REQ_MSG_SIZE];
};

struct response {
  long mtype;
  char mtext[RESP_MSG_SIZE];
};
