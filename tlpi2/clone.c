#define _GNU_SOURCE // use CLONE_FILES
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define CHILD_SIG SIGUSR1

static int childFunc(void *arg) {
  if(close(*((int*)arg)) == -1) {
    perror("close");
    exit(1);
  }

  return 0;
}

int main(int agc, char *argv[]) {
  const int STACK_SIZE = 65536;
  char *stack;
  char *stackTop;
  int s, fd, flags;
  struct sigaction sa;

  fd = open("/dev/null", O_RDWR);
  if(fd == -1) {
    perror("open");
    exit(1);
  }

  stack = malloc(STACK_SIZE);
  if(stack == NULL) {
    perror("malloc");
    exit(1);
  }

  stackTop = stack + STACK_SIZE;

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  if(sigaction(CHILD_SIG, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  if(clone(childFunc, stackTop, CLONE_FILES | CHILD_SIG, (void*)&fd) == -1) {
    perror("clone");
    exit(1);
  }

  if(waitpid(-1, NULL, (CHILD_SIG != SIGCHLD) ? __WCLONE : 0) == -1) {
    perror("waitpid");
    exit(1);
  }

  printf("child has terminated\n");
  
  // try CLONE_FILES
  s = write(fd, "x", 1);
  if(s == -1 && errno == EBADF) {
    printf("file descriptor %d has been closed\n", fd);
  }

  exit(EXIT_SUCCESS);


  

}

/* int clone(int (*fn)(void *), void *child_stack, */
          /* int flags, void *arg, ...  */
          /* [> pid_t *ptid, struct user_desc *tls, pid_t *ctid <] ); */

