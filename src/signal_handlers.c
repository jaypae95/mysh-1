#include "signal_handlers.h"
#include "signal.h"

void catch_sigint(int signalNo)
{
  // TODO: File this!
  signal(SIGINT, SIG_IGN);
}

void catch_sigtstp(int signalNo)
{
  // TODO: File this!
  signal(SIGTSTP, SIG_IGN);
}
