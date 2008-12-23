#ifndef WATCHDOG_H
#define WATCHDOG_H

#include"timer.h"

void WatchdogStartup();
void WatchdogEnable( int frequency );
void WatchdogNotify( INDEX index );
void WatchdogAddFlag( INDEX index );

#endif
