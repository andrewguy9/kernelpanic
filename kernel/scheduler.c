#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"timer.h"
#include"mutex.h"
#include"hal.h"

#define SCHEDULER_QUANTUM 10

/*
 * Scheduler Unit:
 * The Scheduler unit has three tasks:
 * 1) Provide a thread scheduler which
 * picks when threads run.
 * 2) Provide mechanisms to start and stop
 * threads.
 * 3) Provide mechanism to prevent the sceduler
 * from splitting thread level atomic
 * operations.
 *
 * Scheduling next thread:
 * The actual Scheduling occurs in the Schedule
 * function. We must make sure that the Schedule 
 * function does pick a new thread while the 
 * system is in a thread critical section. 
 * We enforce this rule using a mutex, 
 * threads entering a critical section
 * aquire the mutex, and then release it
 * at the end of their critical section. 
 *
 * Resuming and Blocking threads:
 * Resuming threads will require locking the waiting
 * thread 
 */

struct THREAD * ActiveThread;
struct THREAD * NextThread;
struct HEAP ThreadHeap;
struct TIMER SchedulerTimer;

struct MUTEX SchedulerLock;

/*
 * Thread function to start a 
 * critical section. 
 */
void SchedulerStartCritical( )
{
	BOOL aquired = MutexLock( & SchedulerLock );
	ASSERT( aquired, "Start Critical should always stop the scheduler");
}

void SchedulerEndCritical()
{
	ASSERT( MutexIsLocked( & SchedulerLock ), "Critical section not started");
	MutexUnlock( & SchedulerLock );
}

void SchedulerForceSwitch()
{
	//TODO
}

void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( MutexIsLocked( & SchedulerLock ), 
			"Only run from critical section" );
	ASSERT( thread->State == THREAD_STATE_BLOCKED, 
			"Thread not blocked" );
	thread->State = THREAD_STATE_RUNNING;
	thread->Link.WeightedLink.Weight = 0;//TODO MAKE FORMULA BIATCH
	HeapAdd( (struct WEIGHTED_LINK *) thread, & ThreadHeap );
}

void SchedulerBlockThread( )
{
	ASSERT( MutexIsLocked( &SchedulerLock ), 
			"Only block thread from critical section");
	ActiveThread->State = THREAD_STATE_BLOCKED;
}

void Schedule( ) 
{
	ASSERT( HalIsAtomic(), 
			"Only run schedule in interrupt mode");

	//See if we are allowed to schedule (not in crit section)
	if( MutexIsLocked( & SchedulerLock ) )
	{
		//save old thread
		if( ActiveThread->State == THREAD_STATE_RUNNING )
		{
			//TODO COME UP WITH A FORMULA
			HeapAdd( (struct WEIGHTED_LINK *) &ActiveThread,
				  & ThreadHeap );
		}
		//fetch new thread.
		NextThread = ( struct THREAD *) HeapPop( & ThreadHeap );
	}

	//reschedule the scheduler to run.
	TimerRegisterASR( &SchedulerTimer,
			SCHEDULER_QUANTUM,
			Schedule);
}

void SchedulerInit()
{
	//Initialize ActiveThread
	ActiveThread = NULL;//TODO Cannot be null, ever.
	NextThread = NULL;
	//Initialize heap
	HeapInit( & ThreadHeap );
	//Initialize the timer
	TimerRegisterASR( & SchedulerTimer,
		   	0, 
			Schedule );
	//Set up Schedule Resource
	MutexLockInit( & SchedulerLock );
}
