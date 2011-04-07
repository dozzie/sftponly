#ifndef __GET_LOGIN_H
#define __GET_LOGIN_H

#include <stdlib.h>

static inline char *get_login(void)
{
  char *login = getenv("USER");
  if (login == NULL)
    login = getenv("LOGNAME");

  return login;
}

#endif // __GET_LOGIN_H
