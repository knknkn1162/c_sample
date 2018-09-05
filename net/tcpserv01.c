#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define SERV_PORT 50000

#define LISTENQ 50
#define BUF_SIZE 1024

void str_echo(int sockfd);
ssize_t writen(int fd, const void *vptr, size_t n);

int main(int argc, char *argv[]) {
  int listenfd, connfd;
  pid_t pid;
  struct sockaddr_in cliaddr, servaddr;
  struct sigaction sa;

  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_NOCLDWAIT;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, 0);

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
  }

  while(1) {
    socklen_t clilen = sizeof(cliaddr);
    if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) == -1) {
      perror("accept");
      exit(1);
    }

    if((pid = fork()) < 0) {
      perror("fork");
      exit(1);
    } else if(pid == 0) {
      if(close(listenfd) == -1) {
        perror("close");
        exit(1);
      }
      str_echo(connfd);
      if(close(connfd) == -1) {
        perror("close");
        exit(1);
      }

      // 7. SIGCHLD signal is sent to the parent when the server child terminates
      exit(0);
    }
    if(close(connfd) == -1) {
      perror("close");
      exit(1);
    }
  }
}

void str_echo(int sockfd) {
  ssize_t n;
  char buf[BUF_SIZE];
  // when the server TCP receives the FIN, the read returns 0.
  while((n = read(sockfd, buf, BUF_SIZE)) > 0) {
    //fprintf(stderr, "[server] buf: %s\n", buf);
    if(writen(sockfd, buf, n) < 0) {
      if(errno == EINTR) {
        continue;
      }
      exit(1);
    }
  }
  // 5. server exits
  // 6. FIN server->client
  // 6.2 ACK client->server
  // 6.3 connection is completely terminated
  // client: [TIME_WAIT], server: [CLOSED]
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
