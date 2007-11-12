#include"resource.h"
#include"scheduler.h"
#include"../utils/utils.h"
#include"panic.h"

/*
 * Resource locks are a thread safe syncronization mechanism.
 * These are also called duel mode locks. A thread can lock a 
 * resource in two ways:
 * Shared - RESOURCE_SHARED - Multiple threads can hold a lock in shared mode.
 * Exclusive - RESOURCE_EXCLUSIVE - Only one thread can hold the lock exclusive
 *  at a time.
 *
 * Resources are commonly used on buffers where multiple threads are interrested
 * in data. Many threads shoud beable to share read access on a buffer, since 
 * reading does not affect other readers. Write access to a buffer would require
 * exclusive locking since readers would be confused if they saw a partial write.
 *
 * The resource unit enforces these rules by blocking threads which cannot aquire 
 * the lock immediatly. 
 *
 * This unit is starvation safe and should only be called by threads.
 *
 */

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
		else if( state == RESOURCE_EXCLUSIVE )
		{
			 
			if( lock->NumShared == 0 )
			{
				lock->State = RESOURCE_EXCLUSIVE;
			}
			else
			{//We cannot aquire the lock, now block the thread.
				blocked = TRUE;
			}
		}
		else
		{
			KernelPanic( RESOURCE_INVALID_STATE );
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

	ASSERT( state == lock->State, 
			RESOURCE_SHARED_UNLOCK_WRONG_MODE,
			"tried to unlock resource in wrong mode");

	switch( lock->State )
	{
		case RESOURCE_SHARED:

			ASSERT( lock->NumShared > 0,
					RESOURCE_SHARED_UNLOCK_WHEN_UNLOCKED,
					"too many unlocks for resource");
			lock->NumShared--;
			break;
		case RESOURCE_EXCLUSIVE:

			ASSERT( lock->NumShared == 0,
					RESOURCE_SHARED_SHARED_WHILE_EXCLUSIVE,
					"Shared held while exclusive");
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
