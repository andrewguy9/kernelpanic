#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"timer.h"
#include"mutex.h"

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
struct TIMER ScheduleTimer;

struct MUTEX SchedulerLock;

void SchedulerStartCritical( )
{
	BOOL worked = MutexLock( & SchedulerLock );
	ASSERT( worked, "Could not start critical section" );
}

void SchedulerEndCritical()
{
	MutexUnlock( & SchedulerLock );
}

void SchedulerForceSwitch()
{
	//TODO
}

void SchedulerResumeThread( struct THREAD * thread )
{
	//TODO
}

void SchedulerBlockThread( )
{
	//TODO
}

void Schedule( ) 
{
	//TODO
}

void SchedulerInit()
{
	//Initialize ActiveThread
	ActiveThread = NULL;//TODO Cannot be null, ever.
	NextThread = NULL;
	//Initialize heap
	HeapInit( & ThreadHeap );
	//Initialize the timer
	TimerRegisterASR( & ScheduleTimer, 0, Schedule );
	//Set up Schedule Resource
	MutexLockInit( & SchedulerLock );
}
