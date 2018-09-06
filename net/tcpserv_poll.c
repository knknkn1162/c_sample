#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/param.h>
#include <poll.h>
#include <limits.h>


#define BUF_SIZE 64
#define LISTENQ 50


ssize_t writen(int fd, const void *vptr, size_t n);

int main(int argc, char *argv[]) {
  int listenfd, connfd, sockfd;
  char buf[BUF_SIZE];
  struct sockaddr_in cliaddr, servaddr;
  struct pollfd client[FOPEN_MAX];
  int i, maxi;

  printf("fopen_max: %d\n", FOPEN_MAX);

  if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }


  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));

  if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
    perror("bind");
    exit(1);
  }

  if(listen(listenfd, LISTENQ) == -1) {
    perror("listen");
    exit(1);
  }

  client[0].fd = listenfd;
  client[0].events = POLLRDNORM; // normal data can be read
  for(i = 1; i < FOPEN_MAX; i++) {
    client[i].fd = -1;
  }
  maxi = 0;

  while(1) {
    int nready;
    int num;
    fprintf(stderr, "[server] poll ready..\n");
    if((nready = poll(client, maxi+1, -1)) == -1) {
      if(errno == EINTR) {
        fprintf(stderr, "eintr\n");
        continue;
      }
      perror("poll");
      exit(1);
    }

    if(client[0].revents & POLLRDNORM) {
      socklen_t clilen = sizeof(cliaddr);
      if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) == -1) {
        perror("accept");
        exit(1);
      }

      for(i = 1; i < FOPEN_MAX; i++) {
        if(client[i].fd < 0) {
          client[i].fd = connfd;
          fprintf(stderr, "set client.fd in %d\n", i);
          break;
        }
      }

      if(i == FOPEN_MAX) {
        fprintf(stderr, "too many clients\n");
        exit(1);
      }
      client[i].events = POLLRDNORM;
      if(i > maxi) maxi = i;
      if(--nready <= 0) continue;
    }

    for(i = 1; i <= maxi; i++) {
      if((sockfd = client[i].fd) < 0) {
        continue;
      }
      if(client[i].revents & (POLLRDNORM | POLLERR)) {
        if((num = read(sockfd, buf, BUF_SIZE)) < 0) {
          if(errno == ECONNRESET) {
            if(close(sockfd) == -1) {
              perror("close");
              exit(1);
            }
            client[i].fd = -1;
          } else {
            perror("read");
            exit(1);
          }
        } else if (num == 0) {
          char dst[INET_ADDRSTRLEN];
          if(inet_ntop(AF_INET, &cliaddr.sin_addr, dst, INET_ADDRSTRLEN) == NULL) {
            perror("inet_ntop");
            exit(1);
          }
          fprintf(stderr, "[server] reach EOF from %s:%d\n", dst, cliaddr.sin_port);
          if(close(sockfd) == -1) {
            perror("close");
            exit(1);
          }
          client[i].fd = -1;
        } else {
          if(writen(sockfd, buf, num) == -1) {
            perror("writen");
            exit(1);
          }
          if(--nready <= 0) break;
        }
      }
    }
  }
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
