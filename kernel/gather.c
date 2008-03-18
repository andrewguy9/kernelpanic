#include"gather.h"

/*
 * Unit Description
 *
 * Gather is a syncronization mechanism which allow
 * multiple threads to syncronize their actions.
 *
 * Gather is like an inverted semaphore. It is initialized
 * to wait for n threads. Each thread walls GatherSync and is 
 * blocked until all n threads are blocked. Then the lock awakens
 * all of them at the same time. This is useful if you need
 * multiple threads to move in lock step.
 */

/*
 * Initializes the gather structure.
 * Count is the number of threads the gather lock will block.
 * This should not be called on an active gather lock.
 */
void GatherInit( struct GATHER * gather, COUNT count )
{
	gather->Needed = count;
	gather->Present = 0;
	LinkedListInit( & gather->List );
}

/*
 * Call this to syncronize 
 */
void GatherSync( struct GATHER * gather, struct LOCKING_CONTEXT * context )
{
	LockingStart();

	//Mark that new thread has arrived
	gather->Present++;
	//see if everyone is here
	if( gather->Needed == gather->Present )
	{
		//everyone is here
		gather->Present = 0;
		//tell new guy that he can continue
		LockingAcquire( context );
		//Tell everyone waiting they can continue.
		while( ! LinkedListIsEmpty( & gather->List ) )
		{
			struct LOCKING_CONTEXT * curContext;
			curContext = BASE_OBJECT(
					LinkedListPop( &gather->List ),
					struct LOCKING_CONTEXT,
					Link);
			LockingAcquire( curContext );
		}
		//We are done, everyone is awake
		LockingEnd();
	}
	else
	{
		//We are not the last man, 
		//so we need to block.
		LockingBlock( NULL, context );

		//We need to store ourself away for later retreval.
		LinkedListEnqueue( & context->Link.LinkedListLink, & gather->List );

		//We may need to switch threads.	
		LockingSwitch( context );
	}
}
