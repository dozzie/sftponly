#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

#define SFTP_SERVER "/home/dozzie/working/sftponly/sftp-server.so"

int main(int argc, char **argv, char **env)
{
  char *home = "/tmp";

  char *slash;
  if (argc != 3 || strcmp(argv[1], "-c") != 0 ||
      strcmp((slash = strrchr(argv[2], '/')) ? slash + 1 : argv[2],
             "sftp-server") != 0) {
    // nie-sftp
    return 1;
  }

  void *sftps_lib = dlopen(SFTP_SERVER, RTLD_NOW);

  if (sftps_lib == NULL) {
    fprintf(stderr, "dlopen() failed: %s\n", dlerror());
    return 1;
  }

  int (*sftps_main)(int, char **, char **) = dlsym(sftps_lib, "main");

  if (sftps_main == NULL) {
    fprintf(stderr, "dlsym() failed: %s\n", dlerror());
    dlclose(sftps_lib);
    return 1;
  }

  if (chdir(home)) {
    perror("chdir($HOME) failed");
    return 1;
  }

  chroot(".");

  char *args[] = { "sftp-server", NULL };
  int result = sftps_main(1, args, env);

  return result;
}
