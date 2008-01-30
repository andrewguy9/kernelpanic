#include"resource.h"
#include"../utils/utils.h"
#include"panic.h"
#include"blockingcontext.h"
#include"scheduler.h"

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
/*
void ResourceBlockThread( struct RESOURCE * lock, enum RESOURCE_STATE state )
{
	union BLOCKING_CONTEXT context;
	context.ResourceWaitState = state;
	union LINK * link = LockingBlock( & context, NULL );
	LinkedListEnqueue( & link->LinkedListLink, & lock->WaitingThreads );
}
*/
void ResourceWakeThreads( struct RESOURCE * lock )
{
	ASSERT( lock->NumShared == 0,
		   RESOURCE_WAKE_THREADS_NUM_SHARED_NOT_ZERO,
		   "Cant wake threads when shared threads present.");


	ASSERT( ! LinkedListIsEmpty( & lock->WaitingThreads ),
			RESOURCE_WAKE_THREADS_NO_THREADS_TO_WAKE,
			"There is no one to wake here");

	//This function is called when we are going to start 
	//reading threads out of the blocked list.
	//If there are threads in the list, then there 
	//had to be an event which whould require a state change.
	//So we need NumShared to be 0, so we are ready to change.
	struct LOCKING_CONTEXT * cur;
	do
	{
		//Get the locking context at top of queue
		cur = BASE_OBJECT( LinkedListPeek( & lock->WaitingThreads ), 
				struct LOCKING_CONTEXT,
				Link.LinkedListLink );

		if( cur->BlockingContext.ResourceWaitState == RESOURCE_SHARED )
		{//This is a shared thread, pop and see if there are more.
			lock->NumShared++;
			lock->State = RESOURCE_SHARED;
			LinkedListPop( & lock->WaitingThreads );
			LockingAcquire( cur );
		}
		else if( cur->BlockingContext.ResourceWaitState == RESOURCE_EXCLUSIVE )
		{
			if( lock->NumShared == 0 )
			{//First thread was exclusive. Wake him.
				LinkedListPop( & lock->WaitingThreads );
				lock->State = RESOURCE_EXCLUSIVE;
				LockingAcquire( cur );
			}
			else
			{
				//curThread is exclusive, but now NumShared is positive.
				//We cant acquire, so dont wake him. Just break;
			}
			break;
		}
		else
		{
			KernelPanic( RESOURCE_WAKE_THREADS_INVALID_CONTEXT );
		}
	}while(! LinkedListIsEmpty( & lock->WaitingThreads) );
}

//
//Public Functions
//

void ResourceInit( struct RESOURCE * lock )
{
	lock->State = RESOURCE_SHARED;
	LinkedListInit( &lock->WaitingThreads );
	lock->NumShared = 0;
}

void ResourceLockShared( struct RESOURCE * lock )
{
	union BLOCKING_CONTEXT block;

	LockingStart();
	if( ! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		//There are threads already blocking on 
		//this lock, so we need to get in line. 
		block.ResourceWaitState = RESOURCE_SHARED;
		LockingBlock( &block, NULL );
	}
	else if( lock->State == RESOURCE_SHARED )
	{
		//We are in shared mode, and no one is blocking
		//Lets join the party.
		lock->NumShared++;	
		LockingAcquire( NULL );
	}
	else if( lock->State == RESOURCE_EXCLUSIVE )
	{
		//Lock is in exclusive mode, so block
		block.ResourceWaitState = RESOURCE_SHARED;
		LockingBlock( &block, NULL );
	}
	else
	{
		KernelPanic( RESOURCE_LOCK_SHARED_INVALID_SATE );
	}
	LockingSwitch( NULL );
}

void ResourceLockExclusive( struct RESOURCE * lock )
{
	union BLOCKING_CONTEXT block;

	LockingStart();

	if( ! LinkedListIsEmpty( &lock->WaitingThreads ) )
	{
		//There are threads already threads blocking on
		//this lock, so we need to get in line.
		block.ResourceWaitState = RESOURCE_EXCLUSIVE;
		LockingBlock( &block, NULL );
	}
	else if( lock->State == RESOURCE_SHARED )
	{
		if(	lock->NumShared == 0 )
		{
			//The lock is free, so acquire resource exclusive.
			lock->State = RESOURCE_EXCLUSIVE;
			LockingAcquire( NULL );
		}
		else
		{
			//The lock is busy with shared resources.
			block.ResourceWaitState = RESOURCE_EXCLUSIVE;
			LockingBlock( &block, NULL );
		}
	}
	else if( lock->State == RESOURCE_EXCLUSIVE )
	{
			//The lock is already exclusive. Block.
			block.ResourceWaitState = RESOURCE_EXCLUSIVE;
			LockingBlock( &block, NULL );
	}
	else
	{
		KernelPanic( RESOURCE_LOCK_EXCLUSIVE_INVALID_STATE );
	}

	LockingSwitch( NULL );
}

void ResourceUnlockShared( struct RESOURCE * lock )
{
	LockingStart();

	if( lock->State == RESOURCE_SHARED )
	{
		lock->NumShared--;
		if( lock->NumShared == 0 &&
				! LinkedListIsEmpty( & lock->WaitingThreads ))
		{
			//The lock is free, but there are waiting. Activate.
			ResourceWakeThreads( lock );
		}
	}
	else
	{
		KernelPanic( RESOURCE_UNLOCK_SHARED_WRONG_STATE );
	}

	LockingEnd();
}

void ResourceUnlockExclusive( struct RESOURCE * lock )
{

	LockingStart();

	ASSERT( lock->NumShared == 0,
					RESOURCE_UNLOCK_EXCLUSIVE_NUMSHARED_POSITIVE,
					"There should be no shared after exclusive unlock");
	
	if( lock->State == RESOURCE_EXCLUSIVE )
	{
		if( LinkedListIsEmpty( & lock->WaitingThreads ) )
		{
			//No threads to take over, restore to shared
			lock->State = RESOURCE_SHARED;
		}
		else
		{
			//There are threads waiting, use them.
			ResourceWakeThreads( lock );
		}
	}
	else
	{
		//Unlock came unexpectedly.
		KernelPanic( RESOURCE_UNLOCK_EXCLUSIVE_UNLOCK_UNEXPECTED );
	}

	LockingEnd();
}

