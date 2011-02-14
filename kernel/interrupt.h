#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/utils.h"
#include"hal.h"
#include"isr.h"

//
//Unit Management
//

#define InterruptStartup() IsrStartup()

//
//Handle Atomic Sections
//

#define InterruptDisable() IsrDisable(IRQ_LEVEL_TIMER)

#define InterruptEnable() IsrEnable(IRQ_LEVEL_TIMER)

#define InterruptIncrement() IsrIncrement(IRQ_LEVEL_TIMER)

#define InterruptDecrement() IsrDecrement(IRQ_LEVEL_TIMER)

#define InterruptDefer( level, enable ) IsrDefer( level, enable )

//
//Functions for Sanity Checking
//

#ifdef DEBUG
#define InterruptIsAtomic() IsrIsAtomic(IRQ_LEVEL_TIMER)
#define InterruptIsEdge() IsrIsAtomic(IRQ_LEVEL_TIMER)
#endif//DEBUG

#endif
