#include <unistd.h>
#include <sys/fsuid.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#define SG_SIZE NGROUPS_MAX + 1

int main(void)  {
  uid_t ruid, euid, suid;
  gid_t rgid, egid, sgid;
  gid_t suppGroups[SG_SIZE];
  int numGroups;
  int j;

  if(getresuid(&ruid, &euid, &suid) == -1) { 
    perror("getresuid"); 
    exit(1);
  }
  if(getresgid(&rgid, &egid, &sgid) == -1) {
    perror("getresgid");
    exit(1);
  }
  printf("real: %d, effective: %d, supplement: %d\n", ruid, euid, suid);
  printf("real: %d, effective: %d, supplement: %d\n", rgid, egid, sgid);

  numGroups = getgroups(SG_SIZE, suppGroups);
  for(j = 0; j < numGroups; j++) {
    printf("supplement groups: %d\n", suppGroups[j]);
  }

  exit(EXIT_SUCCESS);
}
