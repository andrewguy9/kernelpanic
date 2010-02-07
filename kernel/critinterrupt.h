#ifndef CRIT_INTERRUPT_H
#define CRIT_INTERRUPT_H

#include"../utils/utils.h"
#include"handler.h"
#include"../utils/linkedlist.h"

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

//
//Functions for handlers
//

void CritInterrupt();
void CritInterruptRegisterHandler(
		struct HANDLER_OBJECT * handler,
		HANDLER_FUNCTION foo,
		void * context );

#endif
