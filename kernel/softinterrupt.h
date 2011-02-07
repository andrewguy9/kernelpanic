#ifndef SOFT_INTERRUPT_H
#define SOFT_INTERRUPT_H

#include"interrupt.h"
#include"../utils/utils.h"
#include"handler.h"

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

void SoftInterruptDefer( enum IRQ_LEVEL level, BOOL enable );

//
//Functions for Sanity Checking
//

#ifdef DEBUG
BOOL SoftInterruptIsAtomic();
BOOL SoftInterruptIsEdge();
#endif//DEBUG

//
//Functions for handlers.
//

void SoftInterrupt();
void SoftInterruptRegisterHandler(
		struct HANDLER_OBJECT * handler,
		HANDLER_FUNCTION foo,
		void * context );

#endif
