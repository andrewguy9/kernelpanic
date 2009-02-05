#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/utils.h"

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

//
//Functions for Sanity Checking
//

#ifdef DEBUG
BOOL InterruptIsAtomic();
BOOL InterruptIsEdge();
#endif//DEBUG

#endif
