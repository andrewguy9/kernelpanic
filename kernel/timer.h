#ifndef TIMER_H
#define TIMER_H

#include"../utils/link.h"

typedef unsigned long int TIME;

typedef void (* TIMER_HANDLER) ();

struct TIMER
{
	struct WEIGHTED_LINK Link;
	TIMER_HANDLER Handler;
};

void TimerInit( );

void TimerRegisterASR( struct TIMER * newTimer, TIME wait, TIMER_HANDLER handler );


#endif
