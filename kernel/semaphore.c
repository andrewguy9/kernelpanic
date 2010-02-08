#include"semaphore.h"
#include"../utils/linkedlist.h"

#include<stdio.h>

/*
 * Semaphore Unit Description
 * Implements a semaphore as a thread only syncronization mechanism. 
 *
 * Semaphore is like a mutex but it allows blocking so that it can be 
 * starvation safe. A semaphore is initialized to a specific count.
 *
 * Threads call Up and Down to access the semaphore. 
 * Up increments the count and down decrements the count.
 *
 * Rather than going negative, the semaphore will block the threads
 * which call Down when the count is 0. Calling up will unblock the thread.
 *
 * Threads will be unblocked in the order in which they were blocked.
 */

void SemaphoreInit( struct SEMAPHORE * lock, COUNT count )
{
	LinkedListInit( & lock->WaitingThreads );
	lock->Count = count;
}

void SemaphoreDown( struct SEMAPHORE * lock, struct LOCKING_CONTEXT * context )
{//LOCK
	LockingStart();
	printf("down\n");
	if( lock->Count == 0 )
	{//block the thread
		printf( "\tblocking on lock %p, context %p\n", lock, context );
		union LINK * link = LockingBlock( NULL, context );
		LinkedListEnqueue( &link->LinkedListLink,
			   	&lock->WaitingThreads);
	}
	else
	{
		printf( "\tacquired lock %p\n", lock );
		lock->Count--;
		LockingAcquire( context );	
	}
	LockingEnd();
}

void SemaphoreUp( struct SEMAPHORE * lock )
{//UNLOCK
	struct LOCKING_CONTEXT * context;
	LockingStart();
	printf("up\n");
	if( ! LinkedListIsEmpty( & lock->WaitingThreads ) )
	{
		printf( "\tacquired lock %p\n", lock );
		context = BASE_OBJECT(
				LinkedListPop( &lock->WaitingThreads ),
				struct LOCKING_CONTEXT,
				Link);

		LockingAcquire( context );
	}
	else
	{
		lock->Count++;
	}

	LockingEnd( );
}
