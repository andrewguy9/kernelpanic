#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"interrupt.h"
#include"scheduler.h"
#include"hal.h"

//Variables that the Scheduler shared ONLY with the timer unit.
extern struct THREAD * ActiveThread;
extern struct THREAD * NextThread;

//
//Unit Variables
//

//Keep track of system time.
TIME Time;

//Keep track of timers waiting to execute.
struct HEAP Timers;

//
//Unit Helper Routines
//

//
//Takes expired timers off of the heap,
//and adds them to the queue that will
//be run by InterruptEnd.
//
void QueueTimers( )
{
	ASSERT( InterruptIsAtomic(), 
			TIMER_RUN_TIMERS_MUST_BE_ATOMIC,
			"timers can only be run from interrupt level.");
	Time++;
	while( HeapSize( &Timers) > 0 && 
			HeapHeadWeight( &Timers ) <= Time )
	{
		struct HANDLER_OBJECT * timer = (struct HANDLER_OBJECT *) 
			HeapPop( & Timers );
		timer->Enabled = FALSE;
		InterruptRegisterPostHandler(
				timer,
				timer->Handler,
				timer->Argument);
	}
}

void TimerStartup( )
{
	Time = 0;
	HeapInit( &Timers );
	HalInitClock();
}


void TimerRegister( 
		struct HANDLER_OBJECT * newTimer,
		TIME wait,
		HANDLER_FUNCTION * handler,
		void * argument )
{
	ASSERT( InterruptIsAtomic(), 
			TIMER_REGISTER_MUST_BE_ATOMIC,
			"timer structures can only be \
			added from interrupt level");
	ASSERT( ! newTimer->Enabled,
			TIMER_REGISTER_TIMER_ALREADY_ACTIVE,
			"timers cannot be double registered");

	newTimer->Enabled = TRUE;
	newTimer->Link.WeightedLink.Weight = Time + wait;
	newTimer->Handler = handler;
	newTimer->Argument = argument;
	HeapAdd( (struct WEIGHTED_LINK * ) newTimer, &Timers );
}

TIME TimerGetTime()
{
	TIME value;
	InterruptDisable();
	value = Time;
	InterruptEnable();
	return value;
}

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void) 
{
	//Save state
	HAL_SAVE_STATE

	//Save the stack pointer
	HAL_SAVE_SP( ActiveThread->Stack );

	//reset the clock
    TCNT0 = 0xff-1*16; //1 ms

	//update interrupt level to represent that we are in inerrupt
	InterruptStart();

	//Queue up timers
	QueueTimers( );

	//Restore the interrupt level, 
	//and run the timers that expired.
	InterruptEnd();

	//Check for scheduling event
	if( NextThread != NULL )
	{
		ActiveThread = NextThread;
		NextThread = NULL;
	}

	//Restore stack pointer
	HAL_SET_SP( ActiveThread->Stack );

	//Restore the thread state
	HAL_RESTORE_STATE
}

