#include"signal.h"
#include"../utils/linkedlist.h"
#include"../utils/utils.h"

/*
 * Signal Unit Description
 * Implements a signal as a thread only synchronization mechanism.
 *
 * A signal can be "signaled" or "not signaled"
 *
 * Threads can call SignalWainOnSignal to "wait until some event has happened."
 *
 * Threads can call SignalSet to move a signal to the "signaled" state,
 * or call SignalUnset to move it to the "not singaled" state.
 * both of these routines can be called no matter what the 
 * signal's current state is. 
 */


void SignalInit( struct SIGNAL * signal, BOOL state )
{
	LinkedListInit( & signal->WaitingThreads );
	signal->State = state;
}

void SignalSet( struct SIGNAL * signal )
{
	struct LOCKING_CONTEXT * context;

	LockingStart();
	if( signal->State )
	{
		//
		//We are already in the signaled state.
		//We have no real work to do.
		//

		ASSERT( LinkedListIsEmpty( & signal->WaitingThreads ) );
	}
	else 
	{
		//
		//We are in the unsignaled state. Change state to signaled,
		//and drain the waiting threads.
		//

		while( ! LinkedListIsEmpty( &signal->WaitingThreads ) )
		{
			context = BASE_OBJECT(
					LinkedListPop( &signal->WaitingThreads ),
					struct LOCKING_CONTEXT,
					Link );

			LockingAcquire( context );
		}

		signal->State = TRUE;
	}
	LockingEnd();
}

void SignalUnset( struct SIGNAL * signal )
{
	LockingStart();

	signal->State = FALSE;

	LockingEnd();
}

void SignalWaitForSignal( struct SIGNAL * signal, struct LOCKING_CONTEXT * context ) 
{
	LockingStart();

	if( signal->State )
	{
		//
		//We are already signaled, so the 
		//person calling should be notified right now.
		//

		LockingAcquire( context );
	}
	else 
	{
		//
		//The signal is in the unsignaled state,
		//so we have to add this thread to the list
		//of people waiting for notification.
		//

		union LINK * link = LockingBlock( NULL, context );
		LinkedListEnqueue( &link->LinkedListLink,
				&signal->WaitingThreads );

	}

	LockingEnd();
}

