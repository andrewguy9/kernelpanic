#include"scheduler.h"
#include"../utils/heap.h"
#include"timer.h"
#include"hal.h"
#include"../utils/utils.h"

#define SCHEDULER_QUANTUM 10

struct THREAD * ActiveThread;
struct THREAD * NextThread;
struct HEAP ThreadHeap;
struct TIMER ScheduleTimer;
unsigned char SchedulerEnabled;

void SchedulerStartCritical( )
{
	DISABLE_INTERRUPTS();
	ASSERT( SchedulerEnabled, 
			"Scheduler was not enabled at start of \
			critical section.");
	SchedulerEnabled = FALSE;
	ENABLE_INTERRUPTS();
}

void SchedulerEndCritical()
{
	DISABLE_INTERRUPTS();
	ASSERT( ! SchedulerEnabled, 
			"Scheduler was not disabled at end \
			of Critical section");
	SchedulerEnabled = TRUE;
	ENABLE_INTERRUPTS();
}

BOOL SchedulerIsCritical()
{
	DISABLE_INTERRUPTS();
	BOOL value = SchedulerEnabled;
	ENABLE_INTERRUPTS();
	return value;
}

void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( thread != NULL, "thread cannot be null" );

	SchedulerStartCritical();
	ASSERT( thread->State == THREAD_STATE_BLOCKED,
			"Only activate blocked threads" );
	thread->State = THREAD_STATE_RUNNING;
	HeapAdd( (struct WEIGHTED_LINK *) thread, &ThreadHeap );
	SchedulerEndCritical();
}

void SchedulerBlockThread( )
{
	ASSERT( ActiveThread != NULL, "thread cannot be null" );
	ASSERT( ActiveThread->State == THREAD_STATE_RUNNING, 
			"Cant stop a thread that isn't running." );

	SchedulerStartCritical();
	ActiveThread->State = THREAD_STATE_BLOCKED;
	//TODO: Force a Scheduling event.
	SchedulerEndCritical();
}

void Schedule( ) 
{
	ASSERT( SchedulerIsCritical(), "scheduler must be run atomically");
	
	//If there is an active thread, add him back
	//into the heap with some penalty.
	if( ActiveThread != NULL )
	{
		//Apply penalty.
		ActiveThread->Link.WeightedLink.Weight += 
			ActiveThread->Priority;
		//Add to queue
		HeapAdd( (struct WEIGHTED_LINK *) ActiveThread,
			  &ThreadHeap );
	}
	ActiveThread = NULL;

	//Retrive the next thread off the heap
	NextThread = (struct THREAD *) HeapPop( & ThreadHeap );

	//Schedule Next Scheduling event.
	TimerRegisterASR( & ScheduleTimer,
		   	SCHEDULER_QUANTUM, 
			Schedule );
}

void SchedulerInit()
{
	//Initialize ActiveThread
	ActiveThread = NULL;
	//Initialize heap
	HeapInit( & ThreadHeap );
	//Initialize the timer
	TimerRegisterASR( & ScheduleTimer, 0, Schedule );
	//Enable Scheduler
	SchedulerEnabled = TRUE;
}
