#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <fcntl.h>

//----------------------------------------------------------------------------

#define SFTP_SERVER "/home/dozzie/working/sftponly/sftp-server.so"

//----------------------------------------------------------------------------

static struct passwd user = {
  NULL,       // login
  "x",        // pass
  0,          // UID
  0,          // GID
  "",         // gecos
  "/",        // home
  "/dev/null" // shell
};

// hack, zeby sftp-server nie konczyl dzialania, bo nie moze znalezc wpisu
// w /etc/passwd
struct passwd *getpwuid(uid_t uid)
{
  if (uid == user.pw_uid)
    return &user;
  return NULL;
}

//----------------------------------------------------------------------------

// hack, zeby sftp-server nie wymagal /dev/null (zakladam, ze deskryptory 0,
// 1 i 2 sa otwarte)
void sanitise_stdfd(void)
{
  return;
}

//----------------------------------------------------------------------------

int main(int argc, char **argv, char **env)
{
  // sposob wywolania
  char *slash;
  if (argc != 3 || strcmp(argv[1], "-c") != 0 ||
      strcmp((slash = strrchr(argv[2], '/')) ? slash + 1 : argv[2],
             "sftp-server") != 0) {
    // nie-sftp
    //fprintf(stderr, "Usage outside sftp protocol prohibited\n");
    return 1;
  }

  // nie moge skorzystac z getpwuid() (dopiero to zdefiniowalem na swoj
  // sposob), wiec probuje sie dobrac do danych o uzytkowniku przez $USER albo
  // $LOGNAME
  char *login = getenv("USER");
  if (login == NULL)
    login = getenv("LOGNAME");

  struct passwd *entry;

  if (login == NULL || (entry = getpwnam(login)) == NULL ||
      entry->pw_uid != getuid()) {
    //fprintf(stderr, "Unknown user (UID %d)\n", getuid());
    return 1;
  }

  // pozniej &user bedzie zwracane przez getpwuid()
  user.pw_name = strdup(entry->pw_name);
  user.pw_uid  = entry->pw_uid;
  user.pw_gid  = entry->pw_gid;

  //--------------------------------------------------------------------------
  // zdobywam adres glownej funkcji sftp-servera

  void *sftps_lib = dlopen(SFTP_SERVER, RTLD_NOW);

  if (sftps_lib == NULL) {
    //fprintf(stderr, "dlopen() failed: %s\n", dlerror());
    return 1;
  }

  int (*sftps_main)(int, char **, char **) = dlsym(sftps_lib, "main");

  if (sftps_main == NULL) {
    //fprintf(stderr, "dlsym() failed: %s\n", dlerror());
    dlclose(sftps_lib);
    return 1;
  }

  //--------------------------------------------------------------------------
  // wywolanie wlasciwe

  // cd $HOME
  if (chdir(user.pw_dir)) {
    perror("chdir($HOME) failed");
    return 1;
  }

  chroot(".");

  setuid(getuid());

  char *args[] = { "sftp-server", NULL };
  int result = sftps_main(1, args, env);

  // zupelnie niepotrzebne, ale w dobrym tonie
  free(user.pw_name);

  return result;
}
