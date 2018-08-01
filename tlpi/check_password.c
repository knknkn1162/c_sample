#define _DEFALUT_SOURCE
#define _XOPEN_SOURCE

#include <unistd.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


int main(void) {
  char *username, *password, *encrypted, *p;
  struct passwd *pwd;
  struct spwd *spwd;
  int authOk;
  size_t len;
  long lnmax = sysconf(_SC_LOGIN_NAME_MAX);
  if(lnmax == -1) { lnmax = 256; }

  username = malloc(lnmax);
  if(username == NULL) { perror("malloc"); }

  printf("Username:");
  fflush(stdout);
  if(fgets(username, lnmax, stdin) == NULL) {
    exit(EXIT_FAILURE);
  }

  len = strlen(username);
  if(username[len-1] == '\n') {
    username[len-1] = '\0';
  }

  pwd = getpwnam(username);
  if(pwd == NULL) {
    perror("getpwnam");
  }
  spwd = getspnam(username);
  if(spwd == NULL && errno == EACCES) {
    perror("no permission to getspnam");
  }

  if (spwd != NULL) {
    pwd->pw_passwd = spwd->sp_pwdp;
  }

  password = getpass("Password: ");

  printf("%s %s\n", password, pwd->pw_passwd);
  if (strcmp(pwd->pw_passwd, "*") == 0) { 
    perror("*");
    exit(1);
  }
  encrypted = crypt(password, pwd->pw_passwd);
  for(p = password; *p != '\0';) {
    *p++ = '\0';
  }

  if(encrypted == NULL) {
    perror("crypt");
  }

  if((authOk = strcmp(encrypted, pwd->pw_passwd)) != 0) {
    printf("incorrect\n");
    exit(EXIT_FAILURE);
  }

  printf("Successfully authenticated UID=%ld\n", (long)pwd->pw_uid);

  return 0;

}
