#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"thread.h"
#include"isr.h"
#include"interrupt.h"

/*
 * Timer Unit Description:
 * Timers allow for function calls to be scheduled for specific time
 * in the future. 
 *
 * Timers are registered with the system by calling TimerRegister.
 * When the timer fires, the function and argument provided to TimerRegister
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
		ASSERT( HeapSize( Timers ) == 0,
				TIMER_OVERFLOW_HAD_TIMERS,
				"There sould be no timers on overflow");
				
		temp = Timers;
		Timers = TimersOverflow;
		TimersOverflow = temp;
	}

	while( HeapSize( Timers ) > 0 && 
			HeapHeadWeight( Timers ) <= Time )
	{

		struct HANDLER_OBJECT * handler = BASE_OBJECT( 
				HeapPop(  Timers ),
				struct HANDLER_OBJECT,
				Link );

		struct POST_HANDLER_OBJECT * timer = BASE_OBJECT(
				handler,
				struct POST_HANDLER_OBJECT,
				HandlerObj);

		timer->Queued = FALSE;

		IsrRegisterPostHandler(
				timer,
				handler->Function,
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
}


void TimerRegister( 
		struct POST_HANDLER_OBJECT * newTimer,
		TIME wait,
		HANDLER_FUNCTION * handler,
		void * context )
{

	InterruptDisable();

	ASSERT( ! newTimer->Queued,
			TIMER_REGISTER_TIMER_ALREADY_ACTIVE,
			"timers cannot be double registered");

	//Construct timer
	newTimer->Queued = TRUE;
	newTimer->HandlerObj.Link.WeightedLink.Weight = Time + wait;
	newTimer->HandlerObj.Function = handler;
	newTimer->Context = context;
	//Add to heap
	if( newTimer->HandlerObj.Link.WeightedLink.Weight >= Time )
	{
		HeapAdd( &newTimer->HandlerObj.Link.WeightedLink, Timers );
	}
	else
	{
		//Overflow ocurred
		HeapAdd( &newTimer->HandlerObj.Link.WeightedLink, TimersOverflow);
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

void TIMER0_OVF_vect(void) 
{
	//update interrupt level to represent that we are in inerrupt
	IsrStart();

	//reset the clock
    TCNT0 = 0xff-1*16; //1 ms

	//Queue Timers to run as Post Handlers.
	QueueTimers( );

	//Restore the interrupt level, 
	IsrEnd();
}

