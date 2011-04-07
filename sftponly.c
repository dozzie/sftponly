#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>

#include "get_login.h"

//----------------------------------------------------------------------------

#ifndef LIBCHROOT_PATH
#  define LIBCHROOT_PATH "/usr/local/lib/sftponly/libchroot.so"
#endif

//----------------------------------------------------------------------------

// taken directly from misc.c from OpenSSH sources
void sanitise_stdfd(void)
{
  int nullfd, dupfd;

  if ((nullfd = dupfd = open("/dev/null", O_RDWR)) == -1) {
    fprintf(stderr, "Couldn't open /dev/null: %s", strerror(errno));
    exit(2);
  }
  while (++dupfd <= 2) {
    /* Only clobber closed fds */
    if (fcntl(dupfd, F_GETFL, 0) >= 0)
      continue;
    if (dup2(nullfd, dupfd) == -1) {
      fprintf(stderr, "dup2: %s", strerror(errno));
      exit(3);
    }
  } 
  if (nullfd > 2)
    close(nullfd);
}

//----------------------------------------------------------------------------

int username_correct(void)
{
  char *login = get_login();

  // fail if no login in environment given
  if (login == NULL)
    return 0;

  struct passwd *pwd_entry = getpwnam(login);

  // fail if no /etc/passwd entry for this login
  if (pwd_entry == NULL)
    return 0;

  // fail if login doesn't match real UID (except for when real UID == root)
  if (pwd_entry->pw_uid != getuid() && getuid() != 0)
    return 0;

  // make sure both env variables are set appropriately
  setenv("USER", login, 1);
  setenv("LOGNAME", login, 1);

  return 1;
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // make sure the 0, 1, and 2 file descriptors are properly open
  sanitise_stdfd();

  if (!username_correct()) {
    fprintf(stderr, "Incorrect username provided! You bastard!\n");
    return 1;
  }

  // check call method
  char *slash;
  if (argc != 3 || strcmp(argv[1], "-c") != 0 ||
      strcmp((slash = strrchr(argv[2], '/')) ? slash + 1 : argv[2],
             "sftp-server") != 0) {
    // nie-sftp
    fprintf(stderr, "Usage outside sftp protocol prohibited\n");
    return 1;
  }

  // XXX: If I don't do it, LD_PRELOAD is not passed to exec()ed process, so
  // I temporarily need to become full-blown root.
  // For now, though, I'm validating correctness of $USER/$LOGNAME
  // (username_correct()).
  setuid(0);
  setenv("LD_PRELOAD", LIBCHROOT_PATH, 1);

  execlp(argv[2], argv[2], NULL);

  return 1;
}
