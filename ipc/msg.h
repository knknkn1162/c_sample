#include <limits.h>
#include <unistd.h>
#define SERVER_KEY 0x1aaaaaa1

struct request {
  long clientId;
  char pathName[PATH_MAX];
};
#define REQ_SIZE (sizeof(struct request))

#define RESP_MSG_SIZE 256
#define RESP_SIZE (sizeof(struct response))

struct response {
  long mtype;
  char message[RESP_MSG_SIZE];
};

#define RESP_FAILURE 1
#define RESP_DATA 2
#define RESP_END 3
