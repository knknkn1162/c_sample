#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LISTENQ 50
#define BUF_SIZE 64

int main(int argc, char *argv[]) {
  fd_set rset, rset_tmp;
  socklen_t clilen;
  int client[FD_SETSIZE];
  int listenfd, maxfd, connfd, sockfd;
  struct sockaddr_in cliaddr, servaddr;
  int maxi, i;

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

  maxfd = listenfd + 1;
  maxi = -1;
  for(i = 0; i < FD_SETSIZE; i++) {
    client[i] = -1;
  }
  FD_ZERO(&rset_tmp);
  FD_SET(listenfd, &rset_tmp);

  while(1) {
    int nready;
    rset = rset_tmp;
    char buf[BUF_SIZE];
    // return the number of file descriptors contained in the three returned descriptor sets
    if((nready = select(maxfd, &rset, NULL, NULL, NULL)) == -1) {
      perror("select");
      exit(1);
    }
    if(FD_ISSET(listenfd, &rset)) {
      clilen = sizeof(cliaddr);
      if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) == -1){
        perror("accept");
        exit(1);
      }
      for(i = 0; i < FD_SETSIZE; i++) {
        if(client[i] < 0) {
          client[i] = connfd;
          break;
        }
      }
      if(i == FD_SETSIZE) {
        perror("too many clients");
        exit(1);
      }
      // FD_SET in connfd
      FD_SET(connfd, &rset_tmp);
      if(connfd+1 > maxfd) {
        maxfd = connfd+1;
      }
      if(i > maxi) {
        maxi = i;
      }
      // if no more readable descriptors, continue
      if(--nready <= 0) {
        continue;
      }
    }

    for(i = 0; i <= maxi; i++) {
      int n;
      if((sockfd = client[i]) < 0) continue;
      if(FD_ISSET(sockfd, &rset)) {
        // if client sends EOF
        if((n = read(sockfd, buf, BUF_SIZE)) == 0) {
          close(sockfd);
          FD_CLR(sockfd, &rset_tmp);
          client[i] = -1;
        } else if (n == -1) {
          perror("read");
          exit(1);
        } else {
          if(write(sockfd, buf, n) != n) {
            perror("write");
            exit(1);
          }
          if(--nready <= 0) {
            break;
          }
        }
      }
    }
  } // end select

  return 0;
}
