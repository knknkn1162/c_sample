#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUF_SIZE 128
#define BACK_LOG 10
#define PORT_NUM 50000

// `telnet 127.0.0.1 5000`
int main(int argc, char *argv[]) {
  int serverfd, clientfd;

  struct sockaddr_in servaddr;
  char buf[BUF_SIZE];
  time_t ticks;

  if((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  // what if PORT_NUM=13?? We should exec this program via super-user!
  servaddr.sin_port = htons(PORT_NUM);

  if(bind(serverfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
    perror("bind");
    exit(1);
  }

  if(listen(serverfd, BACK_LOG) == -1) {
    perror("listen");
    exit(1);
  }

  while(1) {
    if((clientfd = accept(serverfd, (struct sockaddr*)NULL, NULL)) == -1) {
      perror("accept");
      exit(1);
    }

    ticks = time(NULL);
    snprintf(buf, BUF_SIZE, "%.24s\r\n", ctime(&ticks));
    if(write(clientfd, buf, strlen(buf)) != strlen(buf)) {
      perror("write");
      exit(1);
    }
  }
}
