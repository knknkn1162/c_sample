#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 4. call signal_handler
void signal_handler(int sig) {
  // 5. do something in the signal_handler
  // 6. exit
}

int main(int argc, char *argv[]) {
  struct sigaction sa;

  // 0: Set signal handler
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = signal_handler;
  if(sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) { // 9. continue forever. You can stop the program by `Ctrl-Q`(SIGQUIT)
    // 2. Sleep until a signal is delivered
    pause(); // 3. catch SIGINT(Ctrl-C, `kill -INT $(this_pid)`).
    // 7. The pause function returns -1.
    // 8. call printf function.
    printf("catch sigint!\n");
  }
}
