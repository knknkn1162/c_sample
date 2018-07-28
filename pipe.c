#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(void) {
  int count, status;
  int child;
  int pipe_fd[2];
  char* msg = "hello world\n";
  char ch;
  if(pipe(pipe_fd) < 0) {
    perror("pipe");
    exit(1);
  }

  if((child = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if (child == 0) {
    printf("This is child\n");
    // close output
    close(pipe_fd[0]);
    while(*msg) {
      // write input-side pipe
      sleep(1);
      write(pipe_fd[1], msg, 1);
      write(STDOUT_FILENO, "-", 1);
      msg++;
   }
    exit(0);
  } else {
    printf("This is parent\n");
    //close input
    close(pipe_fd[1]);
    // read output-side pipe
    //blocking
    while(read(pipe_fd[0], &ch, 1) > 0) {
      write(STDOUT_FILENO, ".", 1);
      //putchar(ch);
    }
    wait(&status);
    putchar('\n');
    exit(0);
  }

  return 0;
}
