#include <sys/inotify.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_LEN (10*(sizeof(struct inotify_event) + NAME_MAX + 1))


int main(int argc, char *argv[]) {
  int fd;
  int i;
  struct inotify_event *ev;

  fd = inotify_init();
  if(fd == -1) {
    perror("inotify_init");
    exit(1);
  }

  for(i = 1; i < argc; i++) {
    int wd;
    if((wd = inotify_add_watch(fd, argv[i], IN_ALL_EVENTS)) == -1) {
      perror("inotify_add_watch");
      exit(1);
    }
    printf("%s .. fd: %d\n", argv[i], wd);
  }

  while(1) {

    char buf[BUF_LEN];
    char *p;
    int numRead;
    if((numRead = read(fd, buf, BUF_LEN)) <= 0) {
      perror("read");
      exit(1);
    }

    printf("parse buf\n");
    for(p = buf; p < buf + numRead;) {
      ev = (struct inotify_event*)p;
      printf("wd: %d mask: %d %s\n", ev->wd, ev->mask, ev->len ? ev->name: "");
      p += sizeof(struct inotify_event) + ev->len;
    }
  }

  return 0;
}
