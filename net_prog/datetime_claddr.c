#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 64
#define LISTENQ 100

int main(int argc, char *argv[]) {
  int listenfd, connfd;
  socklen_t len;
  struct sockaddr_in servaddr, cliaddr;
  char buf[BUF_SIZE];
  time_t ticks;


  if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(13);

  if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
    perror("bind");
    exit(1);
  }

  if(listen(listenfd, LISTENQ) == -1) {
    perror("listen");
  }

  while(1) {
    len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
    if(connfd == -1) {
      perror("accept");
      exit(1);
    }

    printf("connection from %s, port %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, buf, BUF_SIZE), ntohs(cliaddr.sin_port));

    ticks = time(NULL);
    snprintf(buf, BUF_SIZE, "%.24s\r\n", ctime(&ticks));
    if(write(connfd, buf, strlen(buf)) == -1) {
      perror("write");
      exit(1);
    }

    close(connfd);
  }

  return 0;
}
