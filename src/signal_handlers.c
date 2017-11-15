#include "signal_handlers.h"
#include "signal.h"

void catch_sigint(int signalNo)
{
  // TODO: File this!
  printf("\nCtrl+C Ignored!\n");
  signal(SIGINT, SIG_IGN);
  
}

void catch_sigtstp(int signalNo)
{
  // TODO: Filethis!
  printf("\nCtrl + Z Ignored\n");
  signal(SIGTSTP, SIG_IGN);
  
}
