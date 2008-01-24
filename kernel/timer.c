#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"interrupt.h"
#include"hal.h"
#include"thread.h"

/*
 * Timer Unit Description:
 * Timers allow for function calls to be scheduled for specific time
 * in the future. 
 *
 * Timers are registered with the system by calling TimerRegister.
 * When the timer fires, the function and argument provided to TimerRegister
 * are called as a post interrupt handler. (So interrupts will be ENABLED).
 * Its perfectly save to have a timer re-register itself.
 */
//Variables that the Scheduler shares ONLY with the timer unit.
//This is because the timer unit is used to force a context switch.
extern struct THREAD * ActiveThread;
extern struct THREAD * NextThread;

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
 * and adds them to the queue that will
 * be run by InterruptEnd.
 */
void QueueTimers( )
{
	struct HEAP *temp;
	ASSERT( InterruptIsAtomic(), 
			TIMER_RUN_TIMERS_MUST_BE_ATOMIC,
			"timers can only be run from interrupt level.");

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

	while( HeapSize( Timers) > 0 && 
			HeapHeadWeight( Timers ) <= Time )
	{
		struct HANDLER_OBJECT * timer = BASE_OBJECT( 
				HeapPop(  Timers ),
				struct HANDLER_OBJECT,
				Link );
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
		void * argument )
{
	ASSERT( ! newTimer->Enabled,
			TIMER_REGISTER_TIMER_ALREADY_ACTIVE,
			"timers cannot be double registered");

	InterruptDisable();

	//Construct timer
	newTimer->Enabled = TRUE;
	newTimer->Link.WeightedLink.Weight = Time + wait;
	newTimer->Handler = handler;
	newTimer->Argument = argument;
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

	//Check to see if stack is valid.
	ASSERT( ASSENDING( 
				(unsigned int) ActiveThread->StackLow, 
				(unsigned int) ActiveThread->Stack, 
				(unsigned int) ActiveThread->StackHigh ),
			TIMER_HANDLER_STACK_OVERFLOW,
			"stack overflow");
	
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

