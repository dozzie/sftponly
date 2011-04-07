//----------------------------------------------------------------------------
//
// most of the software do expect that /dev/null is always accessible; we'll
// do them good by overwriting open()/open64(): they will return dup()ed FDs
// for each /dev/null or /dev/zero access
//
//----------------------------------------------------------------------------

#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "open_hack.h"

//----------------------------------------------------------------------------

static int (*__orig_open  )(const char *file, int flags, ...);
static int (*__orig_open64)(const char *file, int flags, ...);

static int dev_null = -1;
static int dev_zero = -1;

void __open_hack(void)
{
  __orig_open   = dlsym(RTLD_NEXT, "open");
  __orig_open64 = dlsym(RTLD_NEXT, "open64");
  dev_null = __orig_open("/dev/null", O_RDWR);
  dev_zero = __orig_open("/dev/zero", O_RDWR);
}

//----------------------------------------------------------------------------

#define __CHECK_DEV_NULL_ZERO \
  if (strcmp("/dev/null", file) == 0) \
    return dup(dev_null);             \
  if (strcmp("/dev/zero", file) == 0) \
    return dup(dev_zero);             \
  va_list mode_arg;                   \
  int mode;                           \
  va_start(mode_arg, flags);          \
  mode = va_arg(mode_arg, int);       \
  va_end(mode_arg);

int open(const char *file, int flags, ...)
{
  __CHECK_DEV_NULL_ZERO;

  return __orig_open(file, flags, mode);
}

int open64(const char *file, int flags, ...)
{
  __CHECK_DEV_NULL_ZERO;

  return __orig_open64(file, flags, mode);
}

//----------------------------------------------------------------------------
