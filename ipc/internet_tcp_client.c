#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"


int main(int argc, char *argv[]) {
  int sfd;
  ssize_t num;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  char port[10];

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;

  snprintf(port, 10, "%d", PORT_NUM);
  if(getaddrinfo("localhost", port, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(1);
  }


  return 0;

}
