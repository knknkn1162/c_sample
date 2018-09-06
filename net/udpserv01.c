#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 64

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));

  if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
    perror("bind");
    exit(1);
  }

  while(1) {
    int num;
    socklen_t len;
    char msg[BUF_SIZE];

    fprintf(stderr, "recvfrom ready..\n");
    if((num = recvfrom(sockfd, msg, BUF_SIZE, 0, (struct sockaddr*)&cliaddr, &len)) == -1) {
      perror("recvfrom");
      exit(1);
    } else if(num == 0) {
      fprintf(stderr, "eof");
      continue;
    }

    msg[num] = '\0';
    fprintf(stderr, "[server] port: %d, family: %d\n", ntohs(cliaddr.sin_port), cliaddr.sin_family);
    if(sendto(sockfd, msg, num, 0, (struct sockaddr*)&cliaddr, len) == -1) {
      perror("sendto");
      exit(1);
    }
  }

  return 0;
}
