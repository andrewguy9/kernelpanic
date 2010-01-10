#ifndef SOFT_INTERRUPT_H
#define SOFT_INTERRUPT_H

#include"../utils/utils.h"

//
//Unit Management
//

void SoftInterruptStartup();

//
//Handle Atomic Sections
//

void SoftInterruptDisable();

void SoftInterruptEnable();

void SoftInterruptIncrement();

void SoftInterruptDecrement();

//
//Functions for Sanity Checking
//

#ifdef DEBUG
BOOL SoftInterruptIsAtomic();
BOOL SoftInterruptIsEdge();
#endif//DEBUG

#endif
