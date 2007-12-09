#include"semaphore.h"
#include"scheduler.h"
#include"../utils/linkedlist.h"

/*
 * Semaphore Unit Description
 * Implements a semaphore thread only syncronization mechanism. 
 *
 * Semaphore is like a mutex but it allows blocking so that it can be 
 * starvation safe. A semaphore is initialized to a specific size. 
 *
 * Threads that lock it request a specific count which is reduced from 
 * the semaphore's pool. When the semaphore cannot satisfy the thread,
 * the thread is blocked until enough resourses are freed. 
 *
 * Resources are granted on a first come first serve basis.
 */

void SemaphoreInit( struct SEMAPHORE * lock, COUNT count )
{
	LinkedListInit( & lock->WaitingThreads );
	lock->Count = count;
}

void SemaphoreDown( struct SEMAPHORE * lock )
{//LOCK
	struct THREAD * thread;

	SchedulerStartCritical( );
	if( lock->Count == 0 )
	{//block the thread
		SchedulerBlockThread();
		thread = SchedulerGetActiveThread();
		LinkedListEnqueue( &thread->Link.LinkedListLink,
			   	&lock->WaitingThreads);
		SchedulerForceSwitch();
	}
	else
	{
		lock->Count--;
		SchedulerEndCritical( );
	}
}

void SemaphoreUp( struct SEMAPHORE * lock )
{//UNLOCK
	struct THREAD * thread;
	SchedulerStartCritical();
	if( ! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		thread = BASE_OBJECT( LinkedListPop( & lock->WaitingThreads),
				struct THREAD,
				Link );
		SchedulerResumeThread( thread );
	}
	else
	{
		lock->Count++;
	}
	SchedulerEndCritical();
}
