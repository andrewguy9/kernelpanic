#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/utils.h"
#include"hal.h"

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

void InterruptDefer( enum IRQ_LEVEL level, BOOL enable );

//
//Functions for Sanity Checking
//

#ifdef DEBUG
BOOL InterruptIsAtomic();
BOOL InterruptIsEdge();
#endif//DEBUG

#endif
