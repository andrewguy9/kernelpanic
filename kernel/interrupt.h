#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/utils.h"

enum INTERRUPT_LEVEL { INTERRUPT_LEVEL, INTERRUPT_LEVEL_SOFT, INTERRUPT_LEVEL_CRIT };

//
//Unit Management
//

void InterruptStartup();

//
//Handle Atomic Sections
//

void InterruptDisable();

void InterruptEnable();

void InterruptIncrement();

void InterruptDecrement();

void InterruptDefer( enum INTERRUPT_LEVEL level, BOOL enable );

//
//Functions for Sanity Checking
//

#ifdef DEBUG
BOOL InterruptIsAtomic();
BOOL InterruptIsEdge();
#endif//DEBUG

#endif
