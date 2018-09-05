#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define FD_SETSIZE 30
#define LISTENQ 50

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
    rset = rset_tmp;
    if(select(maxfd, &rset, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(1);
    }
    if(FD_ISSET(listenfd, &rset)) {
      // FD_SET in connfd
    }

    for(i = 0; i <= maxi; i++) {
      if((sockfd = client[i]) < 0) continue;
      if(FD_ISSET(sockfd, &rset)) {
        // each client
      }
    }
  }
}
