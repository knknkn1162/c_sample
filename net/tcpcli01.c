#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUF_SIZE 1024


ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in servaddr;
  char req[BUF_SIZE], resp[BUF_SIZE];

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) == -1) {
    perror("inet_pton");
    exit(1);
  }

  if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))  == -1) {
    perror("connect");
    exit(1);
  }


  while(fgets(req, BUF_SIZE, stdin) != NULL) {
    if(writen(sockfd, req, strlen(req)) == -1) {
      perror("writen");
      exit(1);
    }

    if(readline(sockfd, resp, BUF_SIZE) == 0) {
      perror("readline");
      exit(1);
    }
    if(fputs(resp, stdout) == EOF) {
      perror("fputs");
      exit(1);
    }

  }

}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
  ssize_t n, rc;
  char c, *ptr;

  ptr = vptr;

  for(n = 1; n < maxlen; n++) {
    if((rc = read(fd, &c, 1)) == 1) {
      *ptr++ = c;
      if(c == '\n') break;
    } else if(rc == 0) {
      *ptr = 0;
      return n-1;
    } else {
      if(errno == EINTR) continue;
      return -1;
    }
  }
  *ptr = 0;
  return n;
}


ssize_t writen(int fd, const void *vptr, size_t n) {
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;

  ptr = vptr;
  nleft = n;
  while(nleft > 0) {
    if((nwritten = write(fd, ptr, nleft)) <= 0) {
      if(nwritten < 0 && errno == EINTR) {
        nwritten = 0;
      } else {
        return -1;
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }

  return n;
}
