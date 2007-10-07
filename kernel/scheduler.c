#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
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

struct LINKED_LIST Queue1;
struct LINKED_LIST Queue2;
struct LINKED_LIST * RunQueue;
struct LINKED_LIST * DoneQueue;

struct TIMER SchedulerTimer;
BOOL QuantumExpired;

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
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) thread, DoneQueue );
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
	{//We are allowed to schedule.
		//save old thread
		if( ActiveThread->State == THREAD_STATE_RUNNING )
		{
			//TODO COME UP WITH A FORMULA
			LinkedListEnqueue( (struct LINKED_LIST_LINK *) ActiveThread,
				  DoneQueue);
		}
		//fetch new thread.
		if( LinkedListIsEmpty( RunQueue ) )
		{
			struct LINKED_LIST * temp = RunQueue;
			RunQueue = DoneQueue;
			DoneQueue = temp;
		}
		ASSERT( ! LinkedListIsEmpty( RunQueue ),
			   "there are no threads to run!");
		NextThread = 
			(struct THREAD * ) LinkedListPop( RunQueue );

		//restart the scheduler timer.
		TimerRegisterASR( &SchedulerTimer,
			NextThread->Priority,
			Schedule);
		QuantumExpired = FALSE;
	}
	else
	{
		//mark the quantum as expired.
		QuantumExpired = TRUE;
	}
}

void SchedulerInit()
{
	//Initialize ActiveThread
	ActiveThread = NULL;//TODO Cannot be null, ever.
	NextThread = NULL;
	//Initialize queues
	LinkedListInit( & Queue1 );
	LinkedListInit( & Queue2 );
	RunQueue = & Queue1;
	DoneQueue = & Queue2;
	//Initialize the timer
	TimerRegisterASR( & SchedulerTimer,
		   	0, 
			Schedule );
	//Set up Schedule Resource
	MutexLockInit( & SchedulerLock );
}
