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

void ResourceBlockThread( struct RESOURCE * lock, enum RESOURCE_STATE state )
{
	union BLOCKING_CONTEXT context;
	context.ResourceWaitState = state;
	union LINK * link = LockingBlock( & context );
	LinkedListEnqueue( & link->LinkedListLink, & lock->WaitingThreads );
}

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
	LinkedListInit( & lock->WaitingThreads );
	lock->NumShared = 0;
}

void ResourceLockShared( struct RESOURCE * lock )
{
	SchedulerStartCritical();
	if( ! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		//There are threads already blocking on 
		//this lock, so we need to get in line. 
		ResourceBlockThread( lock, RESOURCE_SHARED );	
		SchedulerForceSwitch();
	}
	else if( lock->State == RESOURCE_SHARED )
	{
		//We are in shared mode, and no one is blocking
		//Lets join the party.
		lock->NumShared++;
		SchedulerEndCritical();
	}
	else if( lock->State == RESOURCE_EXCLUSIVE )
	{
		//Lock is in exclusive mode, so block
		ResourceBlockThread( lock, RESOURCE_SHARED );
		SchedulerForceSwitch();
	}
	else
	{
		KernelPanic( RESOURCE_LOCK_SHARED_INVALID_SATE );
		SchedulerEndCritical();
	}
	//Thread has resumed, we should have acquired the lock shared
	//Make sure lock is consistant
	ASSERT( lock->State == RESOURCE_SHARED && lock->NumShared > 0,
		 RESOURCE_LOCK_SHARED_EXIT_WRONG_STATE,
		 "We should we locked now, but we are in wrong state");
	return;
}

void ResourceLockExclusive( struct RESOURCE * lock )
{
	SchedulerStartCritical();
	if( ! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		//There are threads already threads blocking on
		//this lock, so we need to get in line.
		ResourceBlockThread( lock, RESOURCE_EXCLUSIVE );
		SchedulerForceSwitch();
	}
	else if( lock->State == RESOURCE_SHARED )
	{
		if(	lock->NumShared == 0 )
		{
			//The lock is free, so acquire resource exclusive.
			lock->State = RESOURCE_EXCLUSIVE;
			SchedulerEndCritical();
		}
		else
		{
			//The lock is busy with shared resources.
			ResourceBlockThread( lock, RESOURCE_EXCLUSIVE );
			SchedulerForceSwitch();
		}
	}
	else if( lock->State == RESOURCE_EXCLUSIVE )
	{
			//The lock is already exclusive. Block.
			ResourceBlockThread( lock, RESOURCE_EXCLUSIVE );
			SchedulerForceSwitch();
	}
	else
	{
		KernelPanic( RESOURCE_LOCK_EXCLUSIVE_INVALID_STATE );
		SchedulerEndCritical();
	}

	//We should have acquired the lock exclusive now.
	ASSERT(lock->State == RESOURCE_EXCLUSIVE && lock->NumShared == 0,
			RESOURCE_LOCK_EXCLUSIVE_WRONG_EXIT,
			"lock was in wrong state after acquired");
}

void ResourceUnlockShared( struct RESOURCE * lock )
{
	SchedulerStartCritical();
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
	SchedulerEndCritical();
}

void ResourceUnlockExclusive( struct RESOURCE * lock )
{
	SchedulerStartCritical();

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
	SchedulerEndCritical();
}

void ResourceEscalate( struct RESOURCE * lock )
{
	SchedulerStartCritical();

	ASSERT( lock->State == RESOURCE_SHARED && lock->NumShared > 0,
			RESOURCE_ESCALATE_RESOURCE_NOT_SHARED,
			"Resource needs to be shared when escalating");

	lock->NumShared--;
	if( lock->NumShared == 0 )
	{
		//check and see if people are in line
		if( LinkedListIsEmpty( & lock->WaitingThreads ) )
		{
			//No other threads using lock,
			//go ahead and escalate.
			lock->State = RESOURCE_EXCLUSIVE;
			ASSERT( lock->NumShared == 0,
					RESOURCE_ESCALATE_NUM_SHARED_NOT_ZERO,
					"if we switched into exclusive, numshared = 0");
			SchedulerEndCritical();
		}
		else
		{
			//other threads are waiting, 
			//wake them up and block
			ResourceBlockThread(  lock, RESOURCE_EXCLUSIVE );
			ResourceWakeThreads( lock );
			SchedulerForceSwitch();
		}
	}
	else
	{
		//threads still using lock, block
		ResourceBlockThread(  lock, RESOURCE_EXCLUSIVE );
		SchedulerForceSwitch();
	}


	//At this point we should have the lock exclusive
	ASSERT( lock->State == RESOURCE_EXCLUSIVE && lock->NumShared == 0,
			RESOURCE_ESCALATE_WRONG_EXIT_STATE,
			"wrong state on escalate exit");
}

void ResourceDeescalate( struct RESOURCE * lock )
{
	SchedulerStartCritical();

	ASSERT( lock->State == RESOURCE_EXCLUSIVE && lock->NumShared == 0,
			RESOURCE_DEESCALATE_RESOURCE_NOT_EXCLUSIVE,
			"Resource needs to be exclusive inorder to deescalate");

	if( LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		//No others in line, so deescalate now.
		lock->State = RESOURCE_SHARED;
		lock->NumShared++;
		ASSERT( lock->NumShared == 1,
				RESOURCE_DEESCALATE_RESOURCE_INCONSISTANT,
				"Resource deescalte state inconsistant");
		SchedulerEndCritical();
	}
	else
	{
		//Threads are already waiting, try to wake them
		ResourceWakeThreads( lock );
		if( lock->State == RESOURCE_SHARED && LinkedListIsEmpty( & lock->WaitingThreads ) )
		{
			//we can join in with other shared threads
			lock->NumShared++;
			SchedulerEndCritical();
		}
		else
		{
			//we cant take lock in shared, so block
			ResourceBlockThread( lock, RESOURCE_SHARED );
			SchedulerForceSwitch();

			ASSERT( lock->State == RESOURCE_SHARED && lock->NumShared > 0,
					RESOURCE_DEESCALATE_BLOCK_STATE_BAD,
					"thread got blocked and came in at wrong state");
		}
	}
}

