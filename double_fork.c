#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

int main(void) {
  pid_t pid, ppid;

  if((pid = fork()) == 0) {
    if((pid = fork()) > 0) {
      exit(0);
    } else {
      // parent process is init
      /* while(getppid() != 1) {  */
        /* fprintf(stdout, "%i\n", getppid()); */
        /* sleep(1); */
      /* } */
      sleep(2);
      fprintf(stdout, "grandparent\n");
      exit(0);
    }
  }

  // wait first child
  if(waitpid(pid, NULL, 0) != pid)  {
    perror("waitpid_error");
  }

  return 0;

}
