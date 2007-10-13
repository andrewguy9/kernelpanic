#include"semaphore.h"
#include"scheduler.h"
#include"../utils/linkedlist.h"

void SemaphoreInit( struct SEMAPHORE * lock, COUNT count )
{
	lock->Count = count;
	LinkedListInit( & lock->WaitingThreads );
}

void SemaphoreLock( struct SEMAPHORE * lock )
{
	SchedulerStartCritical( );
	if( lock->Count > 0 )
	{//We can acquire, there is space left.
		lock->Count --;
		SchedulerEndCritical( );
	}
	else
	{//We cannot acquire, add self to waiting threads.
		//mark ActiveThread is no longer runnable
		SchedulerBlockThread();
		//then add it to the waiting threads list.
		LinkedListEnqueue( (struct LINKED_LIST_LINK *) SchedulerGetActiveThread(), 
				& lock->WaitingThreads);
		//then cause the scheduler to fire, so that this thread is switched out.
		SchedulerForceSwitch();
	}

	//at this point the thread has acquired the lock, continue execution
	return;
}

BOOL SemaphoreLockNonBlocking( struct SEMAPHORE * lock)
{
	SchedulerStartCritical();
	if( lock->Count > 0 )
	{
		lock->Count--;
		SchedulerEndCritical();
		return TRUE;
	}
	else
	{
		SchedulerEndCritical();
		return FALSE;
	}
}

void SemaphoreUnlock( struct SEMAPHORE * lock )
{
	SchedulerStartCritical();
	lock->Count++;
	//Wake threads which were blocking on the lock
	while( lock->Count > 0 &&
			! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		lock->Count--;
		struct THREAD * thread = 
			(struct THREAD *) LinkedListPop( & lock->WaitingThreads );
		SchedulerResumeThread( thread );
	}
	SchedulerEndCritical();
}

