#include"semaphore.h"

void SemaphoreInit( struct SEMAPHORE * lock, COUNT count )
{
	lock->Count = count;
	LinkedListInit( lock->WaitingThreads );
}

void SemaphoreLock( struct SEMAPHORE * lock )
{
	SchedulerStartCritical( );
	if( lock->Count > 0 )
	{//We can acquire, there is space left.
		lock->Count--;
		SchedulerEndCritical( );
	}
	else
	{//We cannot acquire, add self to waiting threads.
		//TODO: we need to mark ActiveThread is no longer runnable,
		//then add it to the waiting threads list.
		//then cause the scheduler to fire, so that this thread is blocked.
	}

	//at this point the thread has acquired the lock, continue execution
	return;
}

void SemaphoreLockNonBlocking( struct SEMAPHORE * lock )
{
	//TODO
}

void SemaphoreUnlock( struct SEMAPHORE * lock )
{
	//TODO
}


