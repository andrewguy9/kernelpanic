#ifndef TIMER_H
#define TIMER_H

#include"../utils/link.h"
#include"../utils/utils.h"
#include"handler.h"

typedef unsigned long int TIME;

//
//Unit Management
//

void TimerStartup( );

//
//Unit Servces
//

TIME TimerGetTime();

void TimerInit( struct HANDLER_OBJECT *newTimer );

void TimerRegister( 
		struct HANDLER_OBJECT * newTimer,
		TIME wait,
		HANDLER_FUNCTION * handler,
		void * argument );

//
//  Timer Handler
//

void TimerInterrupt(void);

#endif
