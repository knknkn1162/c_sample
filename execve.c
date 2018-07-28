#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char** environ;

int main(void) {
  char* argv[] = {"/bin/ls", ".", NULL};
  int status;
  pid_t pid;
  if((pid = fork()) == 0) {
    return execve(argv[0], argv, environ);
  } else if(pid > 0) {
    wait(&status);
    printf("end\n");
  }
  return 0;
}

