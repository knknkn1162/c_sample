#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 10
#define SV_SOCK_PATH "/tmp/ud_ucase"


int main(int argc, char *argv[]) {
  int sfd;
  pid_t pid;
  struct sockaddr_un svaddr, claddr;
  char buf[BUF_SIZE];
  ssize_t numBytes;
  int j;

  memset(&svaddr, 0, sizeof(struct sockaddr_un));
  svaddr.sun_family = AF_UNIX;
  strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) -1);

  if((pid = fork()) == 0) {
    sleep(1);
    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sfd == -1) {
      perror("socket");
      exit(1);
    }


    memset(&claddr, 0, sizeof(struct sockaddr_un));
    claddr.sun_family = AF_UNIX;
    snprintf(claddr.sun_path, sizeof(claddr.sun_path), "%s_cl.%ld", SV_SOCK_PATH, (long)getpid());
    if(bind(sfd, (struct sockaddr*)&claddr, sizeof(struct sockaddr_un)) == -1) {
      perror("bind");
      exit(1);
    }
    printf("[client] end bind\n");

    for(j = 1; j < argc; j++) {
      sleep(1);
      // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
      if(sendto(sfd, argv[j], strlen(argv[j]), 0, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) != strlen(argv[j])) {
        perror("sendto");
      }
      printf("sendto server: %s\n", argv[j]);
      //ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
      numBytes = recvfrom(sfd, buf, BUF_SIZE, 0, NULL, NULL);
      if(numBytes == -1) {
        perror("recvfrom");
      }

      printf("Response %d: %.*s\n", j, (int)numBytes, buf);
    }

    remove(claddr.sun_path);
    _exit(EXIT_SUCCESS);
  


  } else if(pid > 0) {
    //server
    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sfd == -1) {
      perror("socket");
      exit(1);
    }

    if(remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
      perror("remove");
    }

    if(bind(sfd, (struct sockaddr*)&svaddr, sizeof(struct sockaddr_un)) == -1) {
      perror("bind");
      exit(1);
    }
    printf("[server]end bind\n");

    while(1) {
      socklen_t len = sizeof(struct sockaddr_un);
      int j;
      // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
      numBytes = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr*)&claddr, &len);
      if(numBytes == -1) {
        perror("recvfrom");
      }

      printf("server received %ld bytes from %s\n", (long)numBytes, claddr.sun_path);

      for(j = 0; j < numBytes; j++) {
        buf[j] = toupper((unsigned char)buf[j]);
      }

      if(sendto(sfd, buf, numBytes, 0, (struct sockaddr*)&claddr, len) != numBytes) {
        perror("sendto");
        exit(1);
      }
      printf("sendto client: %s\n", buf);
    }
  }

  return 0;
}
