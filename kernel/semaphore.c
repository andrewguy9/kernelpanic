#include"semaphore.h"
#include"scheduler.h"
#include"../utils/linkedlist.h"

void SemaphoreInit( struct SEMAPHORE * lock, COUNT count )
{
	lock->MaxCount = count;
	lock->Count = count;
	LinkedListInit( & lock->WaitingThreads );
}

void SemaphoreLock( struct SEMAPHORE * lock, COUNT count )
{
	ASSERT( count <= lock->MaxCount,
			SEMAHPORE_LOCK_COUNT_TOO_LARGE,
			"We cannot lock the semaphore for more \
			than its granted");

	SchedulerStartCritical( );

	if( lock->Count >= count )
	{//We can acquire, there is enough space.
		lock->Count -= count;
		SchedulerEndCritical( );
	}
	else
	{
		//We cannot acquire lock
		//block the thread and save our 
		//remaining request to the blocking context.
		SchedulerBlockThread( );
		union BLOCKING_CONTEXT * context = SchedulerGetBlockingContext();
		context->SemaphoreCountNeeded = count - lock->Count;
		lock->Count = 0;
		//then add it to the waiting threads list.
		LinkedListEnqueue( (struct LINKED_LIST_LINK *) SchedulerGetActiveThread(), 
				& lock->WaitingThreads);
		//then cause the scheduler to fire, so that this thread is switched out.
		SchedulerForceSwitch();
	}
	//at this point the thread has acquired the lock, continue execution
	return;
}

void SemaphoreUnlock( struct SEMAPHORE * lock, COUNT count )
{
	SchedulerStartCritical();
	lock->Count+=count;

	ASSERT( lock->Count <= lock->MaxCount,
			SEMAPHORE_UNLOCK_COUNT_TOO_LARGE,
			"Somehow count exceeded max count");
	//wake as many threads as we can...
	while( LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		struct THREAD * thread = (struct THREAD *) LinkedListPop( & lock->WaitingThreads );
		if( lock->Count >= thread->BlockingContext.SemaphoreCountNeeded )
		{//The thread can be awakened, there is room.
			//remove count needed from pool.
			lock->Count -= thread->BlockingContext.SemaphoreCountNeeded;
			//the thread is satisfied, clear count needed
			thread->BlockingContext.SemaphoreCountNeeded = 0;
			//awaken the thread
			SchedulerResumeThread( thread );
		}
		else
		{//there is not enough room for this thread
			break;
		}
	}
	SchedulerEndCritical();
}
