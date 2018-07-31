#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
  close(STDIN_FILENO);

  //int fcntl(int fd, int cmd, ... /* arg */ );
  int fd = fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 0);
  printf("%d\n", fd);
  // get file descriptor
  int new_fd = fcntl(STDOUT_FILENO, F_GETFD);
  printf("%d\n", new_fd);

  int accmode = fcntl(STDOUT_FILENO, F_GETFL);
  printf("%d\n", accmode & O_ACCMODE);

  if((accmode & O_ACCMODE) == O_RDONLY) {
    printf("readonly");
  } else if ((accmode & O_ACCMODE) == O_WRONLY) {
    printf("writeonly\n");
  } else if ((accmode & O_ACCMODE) == O_RDWR) {
    printf("read write\n");
  }

  fd = open("./Dockerfile", O_RDONLY | O_CLOEXEC, S_IRUSR);
  printf("file descriptor: %d\n", fd);
  int res = fcntl(fd, F_GETOWN);
  printf("process id: %d\n", res);

  return 0;
}
