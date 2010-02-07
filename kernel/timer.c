#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"thread.h"
#include"interrupt.h"
#include"softinterrupt.h"
#include"hal.h"

/*
 * Timer Unit Description:
 * The timer unit keeps time for the kernel. The current time can be queried 
 * by calling TimerGetTime(). 
 *
 * Timers can be registered with the system by calling TimerRegister().
 * When the timer fires, the function and argument provided to TimerRegister()
 * are called as a post interrupt handler. (So interrupts will be ENABLED).
 * Its perfectly safe to have a timer re-register itself.
 */

//
//Unit Variables
//

//Keep track of system time.
TIME Time;

//Keep track of timers waiting to execute.
struct HEAP TimerHeap1;
struct HEAP TimerHeap2;

struct HEAP * Timers;
struct HEAP * TimersOverflow;

//
//Unit Helper Routines
//

/*
 * Takes expired timers off of the heap,
 * and adds them to the PostInterruptHandlers
 * list.
 */
void QueueTimers( )
{
	struct HEAP *temp;

	Time++;

	if( Time == 0 )
	{//Overflow occured, switch heaps

		//There should be no timers left when we overflow.
		ASSERT( HeapSize( Timers ) == 0 );
				
		temp = Timers;
		Timers = TimersOverflow;
		TimersOverflow = temp;
	}

	while( HeapSize( Timers ) > 0 && 
			HeapHeadWeight( Timers ) <= Time )
	{

		struct HANDLER_OBJECT * timer = BASE_OBJECT( 
				HeapPop(  Timers ),
				struct HANDLER_OBJECT,
				Link );

		//Mark timer as running since its dequeued.
		HandlerRun( timer );

		SoftInterruptRegisterHandler(
				timer,
				timer->Function,
				timer->Context);
	}
}

void TimerStartup( )
{
	Time = 0;
	HeapInit( &TimerHeap1 );
	HeapInit( &TimerHeap2 );

	Timers = &TimerHeap1;
	TimersOverflow = &TimerHeap2;

	HalInitClock();
}

void TimerRegister( 
		struct HANDLER_OBJECT * newTimer,
		TIME wait,
		HANDLER_FUNCTION * handler,
		void * context )
{

	InterruptDisable();

	//Construct timer
	HandlerRegister( newTimer );
	newTimer->Link.WeightedLink.Weight = Time + wait;
	newTimer->Function = handler;
	newTimer->Context = context;

	//Add to heap
	if( newTimer->Link.WeightedLink.Weight >= Time )
	{
		HeapAdd( &newTimer->Link.WeightedLink, Timers );
	}
	else
	{
		//Overflow ocurred
		HeapAdd( &newTimer->Link.WeightedLink, TimersOverflow);
	}
	InterruptEnable();
}

TIME TimerGetTime()
{
	TIME value;
	InterruptDisable();
	value = Time;
	InterruptEnable();
	return value;
}

void TimerInterrupt(void) 
{
	//update interrupt level to represent that we are in inerrupt
	InterruptIncrement();

	//reset the clock
	HalResetClock();

	//Queue Timers to run as Post Handlers.
	QueueTimers( );

	//Restore the interrupt level, 
	InterruptDecrement();
}

