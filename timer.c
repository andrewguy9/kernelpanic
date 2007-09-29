#include"timer.h"
#include"utils/utils.h"
#include"utils/heap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

//Keep track of system time for timers.
TIME Time;

//Keep track of timers waiting to execute.
struct HEAP Timers;

void TimerInit( )
{
	Time = 0;
	HeapInit( &Timers );
}

void TimerRegisterASR( struct TIMER * newTimer, TIME wait, TIMER_HANDLER handler )
{
	ASSERT( newTimer != NULL, "null argument" );
	newTimer->Link.Weight = Time + wait;
	HeapAdd( (struct WEIGHTED_LINK * ) newTimer, &Timers );
}

void RunTimers( )
{
	Time++;
	while( HeapSize( &Timers) > 0 && 
			HeapHeadWeight( &Timers ) >= Time )
	{
		struct TIMER * timer = (struct TIMER *) HeapPop( & Timers );
		timer->Handler();
	}
}

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void) 
{
	//Call the timer utility
	RunTimers( );

	//reset the clock
    TCNT0 = 0xff-1*16; //1 ms
}

