#ifndef WATCHDOG_H
#define WATCHDOG_H

#include"timer.h"

void WatchdogStartup(char desiredMask, int frequency);
void WatchdogNotify( INDEX index );

#endif
