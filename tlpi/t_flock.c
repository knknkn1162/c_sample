#include <sys/file.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
    int fd, lock;
    const char *lname;

    if (argc < 3 || strcmp(argv[1], "--help") == 0 ||
            strchr("sx", argv[2][0]) == NULL)
        perror("%s file lock [sleep-time]\n"
                 "    'lock' is 's' (shared) or 'x' (exclusive)\n"
                 "        optionally followed by 'n' (nonblocking)\n"
                 "    'sleep-time' specifies time to hold lock\n");

    lock = (argv[2][0] == 's') ? LOCK_SH : LOCK_EX;
    if (argv[2][1] == 'n')
        lock |= LOCK_NB;

    fd = open(argv[1], O_RDONLY);               /* Open file to be locked */
    if (fd == -1) {
      perror("open");
      exit(1);
    }

    lname = (lock & LOCK_SH) ? "LOCK_SH" : "LOCK_EX";

    printf("PID %ld: requesting %s\n", (long) getpid(), lname);

    if (flock(fd, lock) == -1) {
        if (errno == EWOULDBLOCK) {
          printf("PID %ld: already locked - bye!\n", (long) getpid());
          exit(1);
        } else {
          printf("flock (PID=%ld)\n", (long) getpid());
          exit(1);

    }}

    printf("PID %ld: granted    %s\n", (long) getpid(), lname);

    sleep((argc > 3) ? atoi(argv[3]): 10);

    printf("PID %ld: releasing  %s\n", (long) getpid(), lname);
    if (flock(fd, LOCK_UN) == -1) {
        perror("flock");
    }

    exit(EXIT_SUCCESS);
}
