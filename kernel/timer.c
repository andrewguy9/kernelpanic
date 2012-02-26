#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"thread.h"
#include"isr.h"
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

//TODO WE SHOULD REFACTOR TIME INTO A TIME UNIT, AND HAVE TIMERS
//BE IN THIS UNIT.

//
//  Prototypes
//

void TimerInterrupt(void);
void QueueTimers(void);
void TimerSetNextTimer(void);

//
//Unit Variables
//

//Keep track of system time.
TIME Time;
TIME TimerLastTime;

//Keep track of timers waiting to execute.
//We protect the timer heap with IRQ_LEVEL_MAX.
struct HEAP TimerHeap1;
struct HEAP TimerHeap2;

struct HEAP * Timers;
struct HEAP * TimersOverflow;

//
//Unit Helper Routines
//

/*
 * Takes expired timers off of the heap,
 * and queues them as SoftInterrupts
 * list.
 */
void QueueTimers( )
{
	struct HEAP *temp;

	TimerLastTime = Time;
	Time = HalGetTime();

	if( Time < TimerLastTime )
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

	//Now that we have de-queued all the fired timers,
	//lets calculate when the next hardware interrupt should be.
	TimerSetNextTimer();
}

void TimerSetNextTimer() 
{
	if(HeapSize(Timers) > 0) {
		TIME nextTimer = HeapHeadWeight( Timers );
		TIME delta = nextTimer - Time;
		HalSetTimer(delta);
	} else {
		//If there are no timers in the Timers heap,
		//then we know that the next time to wake will be 
		//an unknown number after the next overflow. 
		TIME rollover = -1 - Time;
		HalSetTimer(rollover + 1);
	}
}

void TimerStartup( )
{
	HalInitClock();
	Time = HalGetTime();
	TimerLastTime = 0;

	HeapInit( &TimerHeap1 );
	HeapInit( &TimerHeap2 );

	Timers = &TimerHeap1;
	TimersOverflow = &TimerHeap2;

	HalRegisterIsrHandler( TimerInterrupt, (void *) HAL_ISR_TIMER, IRQ_LEVEL_TIMER );
	TimerSetNextTimer();
}

void TimerRegister( 
		struct HANDLER_OBJECT * newTimer,
		TIME wait,
		HANDLER_FUNCTION * handler,
		void * context )
{
	TIME timerTime;

	IsrDisable(IRQ_LEVEL_MAX);

	//Construct timer
	HandlerRegister( newTimer );
	timerTime = Time + wait;
	newTimer->Function = handler;
	newTimer->Context = context;

	//Add to heap
	if( timerTime >= Time )
	{
		HeapAdd(timerTime, &newTimer->Link.WeightedLink, Timers );
	}
	else
	{
		//Overflow ocurred
		HeapAdd(timerTime, &newTimer->Link.WeightedLink, TimersOverflow);
	}

	IsrEnable(IRQ_LEVEL_MAX);
	//Because we added a new timer, we may want to wait
	//a different amount of time than previously thought.
	//Lets update the hardware countdown.
	TimerSetNextTimer();

}

TIME TimerGetTime()
{
        TIME value;
        value = Time;
        return value;
}

void TimerInterrupt(void)
{
	//update interrupt level to represent that we are in inerrupt
	TimerIncrement();

	//reset the clock
	HalResetClock();

	//Queue Timers to run as Post Handlers.
	QueueTimers( );

	//Restore the interrupt level, 
	TimerDecrement();
}

