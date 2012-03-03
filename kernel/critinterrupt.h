#ifndef CRIT_INTERRUPT_H
#define CRIT_INTERRUPT_H

#include"../utils/utils.h"
#include"handler.h"
#include"isr.h"

//
//Unit Management
//

void CritInterruptStartup();

//
//Handle Atomic Sections
//

#define CritInterruptDisable() IsrDisable(IRQ_LEVEL_CRIT)

#define CritInterruptEnable() IsrEnable(IRQ_LEVEL_CRIT)

#define CritInterruptIncrement() IsrIncrement(IRQ_LEVEL_CRIT)

#define CritInterruptDecrement() IsrDecrement(IRQ_LEVEL_CRIT)

#define CritInterruptDefer( level, enable ) IsrDefer(level, enable)

//
//Functions for Sanity Checking
//

#ifdef DEBUG
#define CritInterruptIsAtomic() IsrIsAtomic(IRQ_LEVEL_CRIT)
#define CritInterruptIsEdge() IsrIsEdge(IRQ_LEVEL_CRIT)
#endif//DEBUG

//
//Functions for handlers
//

void CritInterruptRegisterHandler(
                struct HANDLER_OBJECT * handler,
                HANDLER_FUNCTION foo,
                void * context );

#endif
