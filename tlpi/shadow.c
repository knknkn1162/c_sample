#include <stdio.h>
#include <pwd.h>
#include <shadow.h>

int main(void) {
  struct passwd* pwd;
  struct spwd* swd;

  printf("* /etc/passwd\n");
  while((pwd = getpwent()) != NULL) {
    printf("%s %d\n", pwd->pw_name, pwd->pw_uid);
  }

  printf("* /etc/shadow\n");
  while((swd = getspent()) != NULL) {
    printf("%s pass: %s\n", swd->sp_namp, swd->sp_pwdp);
  }
}
