#ifndef TIMER_H
#define TIMER_H

#include"../utils/link.h"
#include"../utils/utils.h"
#include"isr.h"

typedef unsigned long int TIME;

//
//Unit Management
//

void TimerStartup( );

//
//Unit Servces
//

TIME TimerGetTime();

void TimerRegister( 
		struct POST_HANDLER_OBJECT * newTimer,
		TIME wait,
		HANDLER_FUNCTION * handler,
		void * argument );


#endif
