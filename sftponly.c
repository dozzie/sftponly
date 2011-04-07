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

#ifndef SUBSYSTEM_SCP
#  define SUBSYSTEM_SCP "/usr/bin/scp"
#endif
#ifndef SUBSYSTEM_RSYNC
#  define SUBSYSTEM_RSYNC "/usr/bin/rsync"
#endif
#ifndef SUBSYSTEM_SFTP
// RHEL:   /usr/libexec/openssh/sftp-server
// Debian: /usr/lib/openssh/sftp-server
#  define SUBSYSTEM_SFTP "/usr/lib/openssh/sftp-server"
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

#define MAX_TOKENS 1024
#define TOKEN_DELIM " \t"

void tokenize_string(const char *str, char ***split)
{
  char *buffer = strdup(str);
  char *token = NULL;
  *split = calloc(MAX_TOKENS, sizeof(char*));

  (*split)[0] = strtok_r(buffer, TOKEN_DELIM, &token);

  // XXX: last pointer should be NULL
  for (int i = 1; i < MAX_TOKENS - 1; ++i) {
    (*split)[i] = strtok_r(NULL, TOKEN_DELIM, &token);
    if ((*split)[i] == NULL)
      break;
  }
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
  if (argc != 3 || strcmp(argv[1], "-c") != 0) {
    fprintf(stderr, "Usage outside file-copying protocol prohibited\n");
    return 1;
  }

  char *subsystem = NULL;
  char **args = NULL;

  if (strncmp(argv[2], "rsync ", 6) == 0) {
    subsystem = SUBSYSTEM_RSYNC;
    tokenize_string(argv[2], &args);
  } else if (strncmp(argv[2], "scp ", 4) == 0) {
    subsystem = SUBSYSTEM_SCP;
    tokenize_string(argv[2], &args);
  } else if (strcmp(argv[2], SUBSYSTEM_SFTP) == 0) {
    subsystem = SUBSYSTEM_SFTP;
    args = calloc(2, sizeof(char *));
    args[0] = subsystem;
  } else {
    fprintf(stderr, "Unrecognized copying protocol: %s\n", argv[2]);
    return 1;
  }

  // XXX: If I don't do it, LD_PRELOAD is not passed to exec()ed process, so
  // I temporarily need to become full-blown root.
  // For now, though, I'm validating correctness of $USER/$LOGNAME
  // (username_correct()).
  setuid(0);
  setenv("LD_PRELOAD", LIBCHROOT_PATH, 1);

  execv(subsystem, args);

  fprintf(stderr, "Couldn't exec(%s): %s", subsystem, strerror(errno));

  return 1;
}
