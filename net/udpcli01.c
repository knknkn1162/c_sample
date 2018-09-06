#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>


#define BUF_SIZE 64

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;
  char req[BUF_SIZE], resp[BUF_SIZE];

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) == -1) {
    perror("inet_pton");
    exit(1);
  }


  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("sockfd");
    exit(1);
  }

  while(fgets(req, BUF_SIZE, stdin) != NULL) {
    int num;
    socklen_t len;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if(sendto(sockfd, req, strlen(req), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
      perror("[client] sendto");
      exit(1);
    }

    if(getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) == -1) {
      perror("getsockname");
      exit(1);
    }

    fprintf(stderr, "[client] sendto %d -> %d\n", ntohs(addr.sin_port), ntohs(servaddr.sin_port));
    if((num = recvfrom(sockfd, resp, BUF_SIZE, 0, (struct sockaddr*)&cliaddr, &len)) == -1) {
      perror("recvfrom");
      exit(1);
    }
    fprintf(stderr, "[client] recvfrom port: %d\n", ntohs(cliaddr.sin_port));

    resp[num] = '\0';
    if(fputs(resp, stdout) == EOF) {
      perror("fputs");
      continue;
    }
  }

  return 0;
}
