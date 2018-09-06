#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/param.h>
#include <signal.h>

#define BUF_SIZE 1024


ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);

volatile sig_atomic_t sigFlag = 0;

void signal_handler(int sig) {
  sigFlag = 1;
}

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in servaddr;
  char buf[BUF_SIZE];
  fd_set rset;
  int maxfd;
  int eof_flag = 0;
  sigset_t newmask, zeromask;
  struct sigaction sa;

  sigemptyset(&zeromask);
  sigemptyset(&newmask);
  sigaddset(&newmask, SIGINT);
  sigprocmask(SIG_BLOCK, &newmask, NULL);

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = signal_handler;
  sa.sa_flags = 0;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }


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

  while(1) {
    int num;
    FD_ZERO(&rset);
    if(eof_flag == 0) {
      FD_SET(STDIN_FILENO, &rset);
    }
    FD_SET(sockfd, &rset);
    maxfd = sockfd + 1;

    fprintf(stderr, "[client] pselect ready..\n");
    if(pselect(maxfd, &rset, NULL, NULL, NULL, &zeromask) == -1) {
      if(errno == EINTR) {
        if(sigFlag) {
          fprintf(stderr, "catch sigint\n");
          break;
        }
        fprintf(stderr, "catch signal\n");
        continue;
      }
      perror("select");
      exit(1);
    }

    if(FD_ISSET(sockfd, &rset)) {
      if((num = read(sockfd, buf, BUF_SIZE)) == 0) {
        if(eof_flag == 1) {
          fprintf(stderr, "[client] sockfd EOF and exit\n");
          break;
        } else {
          perror("[client] read");
          exit(1);
        }
      }
      if(write(STDOUT_FILENO, buf, num) != num) {
        perror("write");
        exit(1);
      }
    }

    if(FD_ISSET(STDIN_FILENO, &rset)) {
      if((num = read(STDIN_FILENO, buf, BUF_SIZE)) == 0) {
        // Ctrl-D EOF end
        eof_flag = 1;
        // close one-half of the TCP connection.
        // We want to send a FIN to the server, telling it we have finished sending data, but leave the socket descriptor open for reading.
        // EOF on input doesn't imply that we have finished reading from the socket!!
        if(shutdown(sockfd, SHUT_WR) == -1) {
          perror("shutdown");
          exit(1);
        }
        fprintf(stderr, "[server] shutdown\n");
        continue;
      }
      if(writen(sockfd, buf, num) == -1) {
        perror("writen");
        exit(1);
      }
    }
  }
  fprintf(stderr, "[client] exit\n");
  return 0;

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
