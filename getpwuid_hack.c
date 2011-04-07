//----------------------------------------------------------------------------
//
// most of the software do expect that /dev/null is always accessible; we'll
// do them good by overwriting open()/open64(): they will return dup()ed FDs
// for each /dev/null or /dev/zero access
//
//----------------------------------------------------------------------------

#define _GNU_SOURCE
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/param.h> // MAXPATHLEN

#include "getpwuid_hack.h"
#include "get_login.h"

//----------------------------------------------------------------------------

static struct passwd __user = {
  NULL,       // login
  "x",        // pass
  0,          // UID
  0,          // GID
  "",         // gecos
  "/",        // home
  "/dev/null" // shell
};

static char __real_home[MAXPATHLEN] = "";

//----------------------------------------------------------------------------

// hack for sftp-server not to finish too early, what is caused by missing
// /etc/passwd entry
struct passwd *getpwuid(uid_t uid)
{
  if (uid == __user.pw_uid)
    return &__user;
  return NULL;
}

struct passwd *getpwnam(const char *name)
{
  if (strcmp(name, __user.pw_name) == 0)
    return &__user;
  return NULL;
}

const char *__sftponly_real_home()
{
  return __real_home;
}

//----------------------------------------------------------------------------

void __getpwuid_hack(void)
{
  // determine username
  char *login = get_login();

  // get /etc/passwd entry for this username
  struct passwd *pwd_entry;
  struct passwd* (*getpwnam)(const char*) = dlsym(RTLD_NEXT, "getpwnam");

  // XXX: root can specify in $USER/$LOGNAME any login valid in the system
  if (login == NULL || (pwd_entry = getpwnam(login)) == NULL ||
      (pwd_entry->pw_uid != getuid() && getuid() != 0)) {
    fprintf(stderr, "Unknown user (UID %d, expected %s(%d))\n",
            getuid(), login, pwd_entry->pw_uid);
    exit(1);
  }

  // try to set real UID to the one got from $USER/$LOGNAME environment
  // variable and to set effective UID to root
  // if root set $USER or $LOGNAME, I can trust this variable
  // if non-root set $USER, he won't have rights to setuid() anyway
  // FIXME: assumption true unless SUID binary gets this loaded
  setreuid(pwd_entry->pw_uid, 0);

  // later on &__user will be returned by getpwuid() and getpwnam()
  __user.pw_name = strdup(pwd_entry->pw_name);
  __user.pw_uid  = pwd_entry->pw_uid;
  __user.pw_gid  = pwd_entry->pw_gid;

  // home directory under chroot() is a root directory, but before going
  // chroot() I'll need the actual path
  strncpy(__real_home, pwd_entry->pw_dir, sizeof(__real_home) - 1);
  __real_home[sizeof(__real_home) - 1] = 0; // make sure it's null terminated
}

//----------------------------------------------------------------------------
