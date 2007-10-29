#include"resource.h"
#include"scheduler.h"
#include"../utils/utils.h"
#include"panic.h"

void ResourceInit( struct RESOURCE * lock )
{
	lock->State = RESOURCE_SHARED;
	LinkedListInit( &lock->WaitingThreads );
	lock->NumShared = 0;
}

void ResourceLock( struct RESOURCE * lock, enum RESOURCE_STATE state )
{
	struct THREAD * thread;

	SchedulerStartCritical();
	
	BOOL blocked = FALSE;
	if( lock->State == RESOURCE_SHARED )
	{
		if( state == RESOURCE_SHARED )
		{
			lock->NumShared++;
		}
		else if( state == RESOURCE_EXCLUSIVE && 
				lock->NumShared == 0 )
		{
			lock->State = RESOURCE_EXCLUSIVE;
		}
		else
		{//We cannot aquire the lock, now block the thread.
			blocked = TRUE;
		}
	}
	else if( lock->State == RESOURCE_EXCLUSIVE )
	{//if lock is exclusive: block new thread
		blocked = TRUE;	
	}
	else
	{
		KernelPanic(RESOURCE_INVALID_STATE);
	}

	if( blocked )
	{
		SchedulerBlockThread();
		SchedulerGetBlockingContext()->ResourceWaitState = state;
		thread = SchedulerGetActiveThread();
		LinkedListEnqueue( (struct LINKED_LIST_LINK*) thread, 
				& lock->WaitingThreads );
		SchedulerForceSwitch();
	}
	else
	{
		SchedulerEndCritical();
	}
}

void ResourceUnlock( struct RESOURCE * lock, enum RESOURCE_STATE state )
{
	SchedulerStartCritical();

	/*ASSERT( state == lock->State, 
			"tried to unlock resource in wrong mode",
			RESOURCE_SHARED_UNLOCK_WRONG_MODE);*/

	switch( lock->State )
	{
		case RESOURCE_SHARED:
			/*
			ASSERT( lock->NumShared > 0,
					"too many unlocks for resource",
					RESOURCE_SHARED_UNLOCK_WHEN_UNLOCKED);*/
			lock->NumShared--;
			break;
		case RESOURCE_EXCLUSIVE:
			lock->State = RESOURCE_SHARED;
			break;
	}

	struct THREAD * nextThread;
	enum RESOURCE_STATE * nextThreadState;
	while( ! LinkedListIsEmpty( & lock->WaitingThreads ) && 
		lock->State == RESOURCE_SHARED )
	{
		nextThread = (struct THREAD *) LinkedListPeek( & lock->WaitingThreads );
		nextThreadState = (enum RESOURCE_STATE*) & nextThread->BlockingContext;
		if( * nextThreadState == RESOURCE_EXCLUSIVE &&
				lock->NumShared != 0 )
		{
			//We cannnot wake nextThread, so break from loop
			break;
		}

		//we can wake thread, so remove from list
		LinkedListPop( & lock->WaitingThreads );

		//add to scheduled thread pool
		SchedulerResumeThread( nextThread );
		
		//apply changes to lock state
		if( nextThreadState == RESOURCE_SHARED )
			lock->NumShared ++;
		else if( nextThreadState == RESOURCE_EXCLUSIVE )
			lock->State = RESOURCE_EXCLUSIVE;
		else
			KernelPanic(RESOURCE_INVALID_STATE); 
	}
	SchedulerEndCritical();
}
