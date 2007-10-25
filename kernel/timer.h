#ifndef TIMER_H
#define TIMER_H

#include"../utils/link.h"
#include"../utils/utils.h"

typedef unsigned long int TIME;

typedef void (TIMER_HANDLER) (void * Argument);

struct TIMER
{
	struct WEIGHTED_LINK Link;
	BOOL Enabled;
	TIMER_HANDLER * Handler;
	void * Argument;
};

void TimerInit( );
TIME TimerGetTime();
void TimerRegister( struct TIMER * newTimer,
		TIME wait,
		TIMER_HANDLER * handler,
		void * argument );


#endif
