#include <limits.h>
#define SERVER_KEY 0x1aaaaaa1

struct request {
  int clientId;
  char pathName[PATH_MAX];
};

#define RESP_MSG_SIZE 8192

struct response {
  long mtype;
  char data[RESP_MSG_SIZE];
};

#define RESP_FAILURE 1
#define RESP_DATA 2
#define RESP_END 3
