#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 10
#define PORT_NUM 50002


int main(int argc, char *argv[]) {
  struct sockaddr_in6 claddr;
  int j, sfd;
  char buf[BUF_SIZE];
  ssize_t numBytes;

  if(fork() == 0) {
    struct sockaddr_in6 svaddr;
    //client
    sleep(1);
    sfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if(sfd == -1) {
      perror("socket");
      exit(1);
    }

    memset(&svaddr, 0, sizeof(struct sockaddr_in6));
    svaddr.sin6_family = AF_INET6;
    // int inet_pton(int af, const char *src, void *dst);
    if(inet_pton(AF_INET6, "::1", &svaddr.sin6_addr) <= 0) {
      perror("inet_pton failed");
      exit(1);
    }
    // host to network short
    svaddr.sin6_port = htons(PORT_NUM);

    for(j = 1; j < argc; j++) {
      if(sendto(sfd, argv[j], strlen(argv[j]), 0, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_in6)) != strlen(argv[j])) {
        perror("sendto");
        exit(1);
      }

      numBytes = recvfrom(sfd, buf, BUF_SIZE, 0, NULL, NULL);
      if(numBytes == -1) {
        perror("recvfrom");
        exit(1);
      }

      printf("response %d: %.*s\n", j-1, (int)numBytes, buf);
    }

    _exit(EXIT_SUCCESS);
  } else {
    char claddrStr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 svaddr;

    sfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if(sfd == -1) {
      perror("socket");
    }
    memset(&svaddr, 0, sizeof(struct sockaddr_in6));
    svaddr.sin6_family = AF_INET6;
    svaddr.sin6_addr = in6addr_any;
    // host to network short
    svaddr.sin6_port = htons(PORT_NUM);

    if(bind(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_in6)) == -1) {
      perror("bind");
    }
    printf("[server] end bind\n");

    while(1) {
      socklen_t len = sizeof(struct sockaddr_in6);
      numBytes = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr*)&claddr, &len);
      if(numBytes == -1) {
        perror("recvfrom");
      }

      // const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
      if(inet_ntop(AF_INET6, &claddr.sin6_addr, claddrStr, INET6_ADDRSTRLEN) == NULL) {
        printf("couldn't convert client address to string\n");
      } else {
        // network to host short
        printf("Server recieved %ld bytes from (%s, %u)\n", (long)numBytes, claddrStr, ntohs(claddr.sin6_port));
      }

      for(j = 0; j < numBytes; j++) {
        buf[j] = toupper((unsigned char)buf[j]);
      }

      if(sendto(sfd, buf, numBytes, 0, (struct sockaddr*)&claddr, len) != numBytes) {
        perror("sendto");
        exit(1);
      }
    }
  }
}
