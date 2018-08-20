#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "msg.h"

#define BUF_MSG_SIZE 256
// length, msg, pid
#define BUF_SIZE (BUF_MSG_SIZE + sizeof(long) + sizeof(size_t))


int pack(void* data, size_t len, char *msg);
int unpack(void* data, size_t *len, pid_t *pid, char **msg, size_t *msgLen);

int main(int argc, char *argv[]) {

  int pfd[2]; // for client -> pipe -> server
  pid_t pid;

  if(argc < 2) {
    fprintf(stderr, "Usage error: ./a.out [pathName]\n");
    exit(1);
  }

  if(pipe(pfd) == -1) {
    perror("pipe");
    exit(1);
  }

  // file descriptors are duplicated on each processes.
  if((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    size_t len;
    char req[BUF_SIZE];
    // child
    if(close(pfd[0]) == -1) {
      perror("[child] close request read fd");
    }

    len = strlen(argv[1]) + sizeof(size_t) + sizeof(long);
    pack(req, len, argv[1]);

    if(write(pfd[1], req, len) != len) {
      perror("[child] write");
      exit(1);
    }

    if(close(pfd[1]) == -1) {
      perror("[child] close request write");
    }

    printf("[child] exit\n");
    _exit(EXIT_SUCCESS);
  } else {
    //parent
    char req[BUF_SIZE];
    char *msg;

    if(close(pfd[1]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    while(1) {
      pid_t pid;
      size_t len, msgLen;
      size_t numRead;
      if((numRead = read(pfd[0], &req, sizeof(req))) == -1) {
        perror("read");
        exit(1);
      } else if (numRead == 0) {
        printf("[parent] EOF\n");
        break;
      } else {
        unpack(req, &len, &pid, &msg, &msgLen);
        // echo
        if(write(STDOUT_FILENO, msg, msgLen) != msgLen) {
          perror("[parent] write");
          exit(1);
        }
        write(STDOUT_FILENO, "\n", 1);
      }
    }

    // end message
    if(close(pfd[0]) == -1) {
      perror("[parent] close");
      exit(1);
    }

    wait(NULL);
    printf("[parent] exit\n");
    exit(EXIT_SUCCESS);
  }
}

int pack(void* data, size_t len, char *msg) {
  len = strlen(msg) + sizeof(size_t) + sizeof(long);
  long pid = (long)getpid();
  memcpy(data, &len, sizeof(size_t));
  data += sizeof(size_t);
  memcpy(data, &pid, sizeof(long));
  data += sizeof(long);
  memcpy(data, msg, strlen(msg));

  return 0;
}

int unpack(void* data, size_t *len, pid_t *pid, char **msg, size_t *msgLen) {
  char* iter = (char*)data;
  memcpy(len, iter, sizeof(size_t));
  iter += sizeof(size_t);
  memcpy(pid, iter, sizeof(long));
  iter += sizeof(long);
  *msg = iter;
  *msgLen = *len - sizeof(size_t) - sizeof(long);
  return 0;
}
