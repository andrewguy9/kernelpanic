#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"hal.h"
#include"scheduler.h"

extern struct THREAD * ActiveThread;
extern struct THREAD * NextThread;


//Keep track of system time for timers.
TIME Time;

//Keep track of timers waiting to execute.
struct HEAP Timers;

void TimerInit( )
{
	Time = 0;
	HeapInit( &Timers );
	HalInitClock();
}

void TimerRegister( struct TIMER * newTimer, 
		TIME wait, 
		TIMER_HANDLER * handler )
{
	ASSERT( HalIsAtomic(), 
			TIMER_REGISTER_MUST_BE_ATOMIC,
			"timer structures can only be \
		   	added from interrupt level");

	newTimer->Link.Weight = Time + wait;
	newTimer->Handler = handler;
	HeapAdd( (struct WEIGHTED_LINK * ) newTimer, &Timers );
}

void RunTimers( )
{
	ASSERT( HalIsAtomic(), 
			TIMER_RUN_TIMERS_MUST_BE_ATOMIC,
			"timers can only be run from interrupt level.");
	Time++;
	while( HeapSize( &Timers) > 0 && 
			HeapHeadWeight( &Timers ) <= Time )
	{
		struct TIMER * timer = (struct TIMER *) HeapPop( & Timers );
		timer->Handler();
	}
}

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void) 
{
	//Save state
	HalSaveState

	//Save the stack pointer
    ActiveThread->Stack = (void *) SP;

	//Call the timer utility
	RunTimers( );

	//Check for scheduling event
	if( NextThread != NULL )
	{
		ActiveThread = NextThread;
		NextThread = NULL;
	}

	//reset the clock
    TCNT0 = 0xff-1*16; //1 ms

	//make sure we are ready to context switch
	HalPrepareRETI();

	//Restore stack
	SP = (int) ActiveThread->Stack;

	HalRestoreState
}

