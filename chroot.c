//----------------------------------------------------------------------------
//
// most of the software do expect that /dev/null is always accessible; we'll
// do them good by overwriting open()/open64(): they will return dup()ed FDs
// for each /dev/null or /dev/zero access
//
//----------------------------------------------------------------------------

#include <sys/types.h>
#include <unistd.h>

#include "getpwuid_hack.h"
#include "open_hack.h"

//----------------------------------------------------------------------------

static void __go_chroot(void) __attribute__((constructor));

void __go_chroot(void)
{
  // initialize stuff
  __getpwuid_hack();
  __open_hack();

  uid_t real_uid = getuid(); // NOTE: call getuid() *after* __getpwuid_hack()
  const char *home_dir = __sftponly_real_home();

  chroot(home_dir);
  setuid(real_uid);
}

//----------------------------------------------------------------------------
