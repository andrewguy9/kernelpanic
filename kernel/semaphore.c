#include"semaphore.h"
#include"blockingcontext.h"
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

void SemaphoreDown( struct SEMAPHORE * lock, struct LOCKING_CONTEXT * context )
{//LOCK
	LockingStart();
	if( lock->Count == 0 )
	{//block the thread
		union LINK * link = LockingBlock( NULL, context );
		LinkedListEnqueue( &link->LinkedListLink,
			   	&lock->WaitingThreads);
	}
	else
	{
		lock->Count--;
		LockingAcquire( context );	
	}
	LockingSwitch( context );
}

void SemaphoreUp( struct SEMAPHORE * lock )
{//UNLOCK
	LockingStart();
	if( ! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		LockingAcquire( 
				BASE_OBJECT( 
					LinkedListPop( & lock->WaitingThreads ), 
					struct LOCKING_CONTEXT, Link ) );
	}
	else
	{
		lock->Count++;
	}
	LockingEnd( );
}
