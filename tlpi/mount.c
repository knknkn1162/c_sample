#include <sys/mount.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  unsigned long flags;
  char *data, *fstype;
  int j, opt;

  flags = 0;
  data = NULL;
  fstype = NULL;

  while((opt = getopt(argc, argv, "o:t:f:")) != -1) {
    switch(opt) {
      case 'o':
        data = optarg;
        break;
      case 't':
        fstype = optarg;
        break;
      case 'f':
        for(j = 0; j < strlen(optarg); j++) {
          switch(optarg[j]) {
            case 'm': flags |= MS_BIND; break;
            case 'r': flags |= MS_RDONLY; break;
            case 'R': flags |= MS_REMOUNT; break;
            default: perror("getopt"); break;
          }
        }
        break;
      default:
        break;
    }
  }
  if(mount(argv[optind], argv[optind + 1], fstype, flags, data) == -1) {
    perror("mount");
  }

  exit(EXIT_SUCCESS);


}
