#include"resource.h"
#include"scheduler.h"
#include"../utils/utils.h"
#include"panic.h"

/*
 * Resource locks are a thread safe syncronization mechanism.
 * These are also called duel mode locks. A thread can lock a 
 * resource in two ways:
 * Shared - RESOURCE_SHARED - Multiple threads can hold a lock in shared mode
 * 	at the same time. 
 * Exclusive - RESOURCE_EXCLUSIVE - Only one thread can hold the lock exclusive
 *  at a time.
 *
 * Resources are commonly used on buffers where multiple threads are interrested
 * in data. Many threads shoud be able to share read access on a buffer, since 
 * reading does not affect other readers. Write access to a buffer would require
 * exclusive locking since readers would be confused if they saw a partial 
 * write.
 *
 * The resource unit enforces these rules by blocking threads which cannot 
 * aquire the lock immediatly. 
 *
 * This unit is starvation safe and should only be called by threads.
 *
 * The system does not store who holds the lock, so calls to this unit
 * are on the honor system. It is up to each thread to make sure it holds the
 * lock before calling ResourceUnlock, or ResourceEscalate or ResourceDeescalate.
 */

//
//Private helper functions
//

void ResourceWakeThreads( struct RESOURCE * lock )
{
	ASSERT(SchedulerIsCritical(),
			RESOURCE_WAKE_THREADS_NOT_CRITICAL,
			"Inorder to handle thread objects we must be critical");

	struct THREAD * nextThread;
	enum RESOURCE_STATE * nextThreadState;
	while( ! LinkedListIsEmpty( & lock->WaitingThreads ) && 
		lock->State == RESOURCE_SHARED )
	{
		nextThread = BASE_OBJECT( 
				LinkedListPeek( & lock->WaitingThreads ),
				struct THREAD,
				Link);
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
		if( * nextThreadState == RESOURCE_SHARED )
			lock->NumShared ++;
		else if( * nextThreadState == RESOURCE_EXCLUSIVE )
			lock->State = RESOURCE_EXCLUSIVE;
		else
			KernelPanic(RESOURCE_INVALID_STATE); 
	}
}

//
//Public facing functions
//

void ResourceInit( struct RESOURCE * lock )
{
	lock->State = RESOURCE_SHARED;
	LinkedListInit( &lock->WaitingThreads );
	lock->NumShared = 0;
}

/*
 * Locks the resource 'lock' in 'state' mode. 
 * threads calling ResourceLock will block until they can
 * obtain the lock.
 */
void ResourceLock( struct RESOURCE * lock, enum RESOURCE_STATE state )
{
	struct THREAD * thread;

	SchedulerStartCritical();
	
	BOOL blocked = FALSE;
	if( lock->State == RESOURCE_SHARED )
	{
		if( state == RESOURCE_SHARED )
		{
			if( LinkedListIsEmpty( & lock->WaitingThreads ) )
			{
				lock->NumShared++;
			}
			else
			{
				//Cant even thought lock is shared, 
				//because someone else if waiting.
				blocked = TRUE;
			}
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
		LinkedListEnqueue( &thread->Link.LinkedListLink, 
				& lock->WaitingThreads );
		SchedulerForceSwitch();
	}
	else
	{
		SchedulerEndCritical();
	}
}

/*
 * Releases lock 'lock' in mode 'state'.
 */
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

	ResourceWakeThreads( lock );

	SchedulerEndCritical();
}

/*
 * If a thread is holding 'lock' in RESOURCE_SHARED mode,
 * a thread can call ResourceEscalate to switch into 
 * exclusive mode without having to unlock. 
 */
void ResourceEscalate( struct RESOURCE * lock )
{
	struct THREAD * thread;

	SchedulerStartCritical();
	ASSERT( lock->State == RESOURCE_SHARED,
			RESOURCE_ESCALATE_NOT_SHARED,
			"The resource must me shared inorder to escate.");

	ASSERT( lock->NumShared > 0,
			RESOURCE_ESCALATE_NO_OWNERS,
			"There are no owners, who is escalating");

	//Reduce the number of owners be 1
	lock->NumShared--;
	if( lock->NumShared > 0 )
	{
		//There are still owners on the lock, so we have to 
		//block on the lock.
		lock->State = RESOURCE_EXCLUSIVE;
	}
	else
	{
		//We are the only owner, so take the lock exclusivly.
		SchedulerBlockThread();
		SchedulerGetBlockingContext()->ResourceWaitState = RESOURCE_EXCLUSIVE;
		thread = SchedulerGetActiveThread();
		LinkedListEnqueue( &thread->Link.LinkedListLink,
				& lock->WaitingThreads );
		SchedulerForceSwitch();
	}
	SchedulerEndCritical();
}

/*
 * A thread holding resource 'lock' in RESOURCE_EXCLUSIVE mode can
 * switch to RESOURCE_SHARED mode by calling ResourceDeescalate.
 */
void ResourceDeescalate( struct RESOURCE * lock )
{
	SchedulerStartCritical();

	ASSERT( lock->State == RESOURCE_EXCLUSIVE,
			RESOURCE_DEESCALATE_NOT_EXCLUSIVE,
			"Resource must be exclusive to de-escalate.");

	ASSERT( lock->NumShared == 0,
			RESOURCE_DEESCALATE_NUM_SHARED_NOT_ZERO,
			"Resource should have no shared holders");

	lock->State = RESOURCE_SHARED;
	lock->NumShared++;

	ResourceWakeThreads( lock );

	SchedulerEndCritical();
}
