#include "signal.h"

void catch_sigint(int)
{
  // TODO: File this!
  printf("11111");
  signal(SIGINT, SIG_IGN);
}

void catch_sigtstp(int);
{
  // TODO: File this!
  signal(SIGTSTP, SIG_IGN);
}
