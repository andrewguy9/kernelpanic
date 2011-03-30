#ifndef TIMER_H
#define TIMER_H

#include"../utils/link.h"
#include"../utils/utils.h"
#include"handler.h"

//
//Unit Management
//

void TimerStartup( );

//
//Unit Servces
//

TIME TimerGetTime();

void TimerRegister( 
		struct HANDLER_OBJECT * newTimer,
		TIME wait,
		HANDLER_FUNCTION * handler,
		void * argument );
//
//Handle Atomic Sections
//

#define TimerDisable() IsrDisable(IRQ_LEVEL_TIMER)

#define TimerEnable() IsrEnable(IRQ_LEVEL_TIMER)

#define TimerIncrement() IsrIncrement(IRQ_LEVEL_TIMER)

#define TimerDecrement() IsrDecrement(IRQ_LEVEL_TIMER)

//
//Functions for Sanity Checking
//

#ifdef DEBUG
#define TimerIsAtomic() IsrIsAtomic(IRQ_LEVEL_TIMER)
#define TimerIsEdge() IsrIsAtomic(IRQ_LEVEL_TIMER)
#endif//DEBUG

#endif
