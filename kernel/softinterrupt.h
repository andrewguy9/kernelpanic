#ifndef SOFT_INTERRUPT_H
#define SOFT_INTERRUPT_H

#include"isr.h"
#include"../utils/utils.h"
#include"handler.h"

//
//Unit Management
//

void SoftInterruptStartup();

//
//Handle Atomic Sections
//

#define SoftInterruptDisable() IsrDisable(IRQ_LEVEL_SOFT)

#define SoftInterruptEnable() IsrEnable(IRQ_LEVEL_SOFT)

#define SoftInterruptIncrement() IsrIncrement(IRQ_LEVEL_SOFT)

#define SoftInterruptDecrement() IsrDecrement(IRQ_LEVEL_SOFT)

#define SoftInterruptDefer( level, enable ) IsrDefer(level, enable )

//
//Functions for Sanity Checking
//

#ifdef DEBUG
#define SoftInterruptIsAtomic() IsrIsAtomic(IRQ_LEVEL_SOFT)
#define SoftInterruptIsEdge() IsrIsEdge(IRQ_LEVEL_SOFT)
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
