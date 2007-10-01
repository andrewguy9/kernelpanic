#include"scheduler.h"
#include"../utils/heap.h"
#include"timer.h"

#define SCHEDULER_QUANTUM 10

struct THREAD * ActiveThread;
struct THREAD * NextThread;
struct HEAP ThreadHeap;
struct TIMER ScheduleTimer;

void SchedulerResumeThreadTH( struct THREAD * thread )
{
	//TODO scheduler must be disabled for this to be called.
	ASSERT( thread != NULL, "thread cannot be null" );
	ASSERT( thread->State == THREAD_STATE_BLOCKED,
		   	"Only activate blocked threads" );
	thread->State = THREAD_STATE_RUNNING;
	HeapAdd( (struct WEIGHTED_LINK *) thread, &ThreadHeap );
}

void SchedulerBlockThreadTH( )
{
	//TODO Scheduler must be disabled for this to be called.
	ASSERT( ActiveThread != NULL, "thread cannot be null" );
	ASSERT( ActiveThread->State == THREAD_STATE_RUNNING, 
			"Cant stop a thread that isn't running." );

	ActiveThread->State = THREAD_STATE_BLOCKED;
	ActiveThread = NULL;
}

void Schedule( ) 
{
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
	TimerRegisterASR( &ScheduleTimer,
		   	SCHEDULER_QUANTUM, 
			Schedule );
}

void SchedulerInit()
{
	//Initialize the timer
	TimerRegisterASR( & ScheduleTimer, 0, Schedule );
	//Initialize ActiveThread
	ActiveThread = NULL;
	//Initialize heap
	HeapInit( & ThreadHeap );
}
