#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/utils.h"
#include"hal.h"

//TODO THIS IS A TEMP BACK WHILE I MOVE OFF DISCRETE LEVELS.
extern volatile COUNT InterruptDisabledCount[IRQ_LEVEL_MAX];

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
