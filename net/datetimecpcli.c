#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 64
// try `./a.out 62.149.2.1`
int main(int argc, char *argv[]) {
  int sockfd;
  char buf[BUF_SIZE+1];
  struct sockaddr_in servaddr;
  int num;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(13); // daytime server
  if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    perror("Inet_pton");
    exit(1);
  }

  if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    perror("connect error");
    exit(1);
  }

  while((num = read(sockfd, buf, BUF_SIZE)) > 0) {

    buf[num] = '\0';
    if(fputs(buf, stdout) == EOF) {
      perror("fputs");
      exit(1);
    }
  }

  return 0;
}
