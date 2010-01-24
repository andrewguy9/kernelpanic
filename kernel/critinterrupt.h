#ifndef CRIT_INTERRUPT_H
#define CRIT_INTERRUPT_H

#include"../utils/utils.h"

//
//Unit Management
//

void CritInterruptStartup();

//
//Handle Atomic Sections
//

void CritInterruptDisable();

void CritInterruptEnable();

void CritInterruptIncrement();

void CritInterruptDecrement();

void CritInterruptDefer();

//
//Functions for Sanity Checking
//

#ifdef DEBUG
BOOL CritInterruptIsAtomic();
BOOL CritInterruptIsEdge();
#endif//DEBUG

#endif
