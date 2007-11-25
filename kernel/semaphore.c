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

void SemaphoreInit( struct SEMAPHORE * lock, int count )
{
	LinkedListInit( & lock->WaitingThreads );
	lock->Count = count;
}

void SemaphoreDown( struct SEMAPHORE * lock )
{//LOCK
	struct THREAD * thread;

	SchedulerStartCritical( );
	lock->Count--;
	if( lock->Count < 0 )
	{//block the thread
		SchedulerBlockThread();
		LinkedListEnqueue( (struct LINKED_LIST_LINK*) thread,
			   	&lock->WaitingThreads);
		SchedulerForceSwitch();
	}
	else
	{
		SchedulerEndCritical( );
	}
}

void SemaphoreUp( struct SEMAPHORE * lock )
{//UNLOCK
	struct THREAD * thread;
	SchedulerStartCritical();
	lock->Count++;
	while( lock->Count > 0 && 
		   	! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		lock->Count--;
		thread = (struct THREAD*) LinkedListPop( & lock->WaitingThreads);
		SchedulerResumeThread( thread );
	}
	SchedulerEndCritical();
}
