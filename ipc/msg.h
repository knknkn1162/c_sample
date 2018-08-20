#include <limits.h>
#define SERVER_KEY 0x1aaaaaa1

struct requestMsg {
  long mtype;
  int clientId;
  char pathName[PATH_MAX];
};

#define REQ_MSG_SIZE (offsetof(struct requestMsg, pathName) - offsetof(struct requestMsg, clientId) + PATH_MAX)

#define RESP_MSG_SIZE 8192

struct responseMsg {
  long mtype;
  char data[RESP_MSG_SIZE];
};

#define RESP_MT_FAILURE 1
#define RESP_MT_DATA 2
#define RESP_MT_END 3
