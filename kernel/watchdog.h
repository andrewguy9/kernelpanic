#ifndef WATCHDOG_H
#define WATCHDOG_H

#include"utils/types.h"

void WatchdogStartup();
void WatchdogEnable( TIME timeout );
void WatchdogNotify( INDEX index );
void WatchdogAddFlag( INDEX index );

#endif
