#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define SV_SOCK_PATH "/tmp/us_xfr"
#define BUF_SIZE 100
#define BACKLOG 5

int main(int argc, char *argv[]) {
   /* struct sockaddr_un { */
    /* sa_family_t sun_family;               [> AF_UNIX <] */
    /* char        sun_path[UNIX_PATH_MAX];  [> pathname <] */
/* }; */
  struct sockaddr_un addr;
  int sfd, cfd;
  ssize_t numRead;
  pid_t pid;

  int fd = open(argv[1], O_RDONLY);
  char buf[BUF_SIZE];
  if(fd == -1) {
    perror("file open");
    exit(1);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if((pid = fork()) == 0) {
    // client
    sleep(1);
    int fd = open(argv[1], O_RDONLY);
    if(fd == -1) {
      perror("fd open client");
    }
    // create socket individually

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sfd == -1) {
      perror("socket");
    }

    if(connect(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
      perror("connect");
      exit(1);
    }

    printf("connect client\n");
    while((numRead = read(fd, buf, BUF_SIZE)) > 0) {
      if(write(sfd, buf, numRead) != numRead) {
        perror("write");
        exit(1);
      }
    }

    if(numRead == -1) {
      perror("read");
      exit(1);
    }

    if(close(fd) == -1) {
      perror("file close");
      exit(1);
    }
    _exit(EXIT_SUCCESS);

  } else if (pid > 0) {
    //server
    int fd = open(argv[2], O_WRONLY);
    if(fd == -1) {
      perror("file open server");
      exit(1);
    }

    // create socket individually
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sfd == -1) {
      perror("socket");
    }

    if(bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
      perror("bind");
      exit(1);
    }

    if(listen(sfd, BACKLOG) == -1) {
      perror("listen");
      exit(1);
    }

    printf("listen server\n");
    while(1) {
      // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
      cfd = accept(sfd, NULL, NULL);
      if(cfd == -1) {
        perror("accept");
      }

      while((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
        if(write(fd, buf, numRead) != numRead) {
          perror("partial/failed write");
          exit(1);
        }
      }
      printf("write fd server\n");

      if(numRead == -1) {
        perror("read");
        exit(1);
      }

      if(close(cfd) == -1) {
        perror("close");
        exit(1);
      }
      if(close(fd) == -1) {
        perror("close file");
        exit(1);
      }
    }
  }
  return 0;
}
