#include <netdb.h>
#include "is_seqnum.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int createAddrInfo(struct addrinfo *info);

int main(int argc, char *argv[]) {
  char seqNumStr[INT_LEN];
  int cfd;
  ssize_t numRead;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  createAddrInfo(&hints);

  if(getaddrinfo(argv[1], PORT_NUM, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  for(rp = result; rp != NULL; rp = rp->ai_next) {

    struct sockaddr_in *addr = (struct sockaddr_in*)rp->ai_addr;
    // int socket(int domain, int type, int protocol); 
    cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    if(cfd == -1) {
      continue;
    }

    // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if(connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1) {

      char str[INET_ADDRSTRLEN];
      // const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
      inet_ntop(AF_INET, &addr->sin_addr, str, INET_ADDRSTRLEN);
      printf("end bind %s:%u\n", str, addr->sin_port);
      break;
    }
    close(cfd);
  }

  if(rp == NULL) {
    perror("could not connect socket to any address");
    exit(1);
  }


  freeaddrinfo(result);

  if(write(cfd, argv[2], strlen(argv[2])) != strlen(argv[2])) {
    perror("write");
    exit(1);
  }

  if(write(cfd, "\n", 1) != 1) {
    perror("write newline");
  }

  numRead = readLine(cfd, seqNumStr, INT_LEN);
  if(numRead == -1) {
    perror("readline");
    exit(1);
  }
  if(numRead == 0) {
    perror("unexpected eof");
    exit(1);
  }

  printf("sequence number: %s\n", seqNumStr);
  exit(EXIT_SUCCESS);

}

int createAddrInfo(struct addrinfo *info) {
  memset(info, 0, sizeof(struct addrinfo));
  info->ai_canonname = NULL;
  info->ai_addr = NULL;
  info->ai_next = NULL;
  info->ai_family = AF_UNSPEC;
  info->ai_socktype = SOCK_STREAM;
  info->ai_flags = AI_NUMERICSERV;
  return 0;
}
