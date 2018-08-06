#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int flags, opt;
  mode_t perms;
  mqd_t mqd;
  struct mq_attr attr, *attrp;

  attrp = NULL;
  attr.mq_maxmsg = 50;
  attr.mq_msgsize = 2048;
  flags = O_RDWR;

  while((opt = getopt(argc, argv, "cm:s:x")) != -1) {
    switch(opt) {
      case 'c':
        flags |= O_CREAT;
        break;
      case 'm':
        attr.mq_maxmsg = atoi(optarg);
        attrp = &attr;
        break;
      case 's':
        attr.mq_msgsize = atoi(optarg);
        attrp = &attr;
      case 'x':
        flags |= O_EXCL;
        break;
      default:
        printf("cant parse\n");
        break;
    }
  }

  if(optind >= argc) {
    perror("optind>=argc");
    exit(1);
  }

  mqd = mq_open(argv[optind], flags, perms, attrp);
  if(mqd == (mqd_t)-1) {
    perror("mq_open");
  }
  printf("%d", (int)mqd);

  exit(EXIT_SUCCESS);
}
