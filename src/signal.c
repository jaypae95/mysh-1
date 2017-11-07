#include "signal.h"

void catch_sigint(int a)
{
  // TODO: File this!
  printf("11111");
  signal(SIGINT, SIG_IGN);
}

void catch_sigtstp(int a);
{
  // TODO: File this!
  signal(SIGTSTP, SIG_IGN);
}
