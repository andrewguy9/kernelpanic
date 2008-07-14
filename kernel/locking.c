#include"locking.h"
#include"scheduler.h"
#include"panic.h"

/*
 * Unit Description
 *
 * Locking unit provides a locking state machine which
 * can be used to various locks and lock consumers to
 * block appropriately.
 */

//
//Helper Routines
//

struct LOCKING_CONTEXT * LockingGetContext( struct LOCKING_CONTEXT * context )
{
	if( context == NULL )
	{
		//Context needs to be fetched from active thread.
		context = SchedulerGetLockingContext();
	}

	return context;
}

//
//Init Routines
//

/*
 * Initialize a locking context for use.
 * NOT ATOMIC
 */
void LockingInit( struct LOCKING_CONTEXT * context, BLOCK_FUNCTION * block, WAKE_FUNCTION * wake )
{
	context->State = LOCKING_STATE_READY;
	context->BlockFunction = block;
	context->WakeFunction = wake;
}

//
//Start Routines
//

/*
 * Locks should call LockingStart to start a critical section.
 */
void LockingStart()
{
	SchedulerStartCritical();
}

//
//State Routines
//

/*
 * Locks should call LockingAcquire to update the statemachine to
 * reflect that the thread has taken the lock. This call should be 
 * surrounded by LockingStart() and LockingEnd/LockingSwitch
 */
void LockingAcquire( struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical() );
	
	context = LockingGetContext(context);

	ASSERT( context->State == LOCKING_STATE_READY || context->State == LOCKING_STATE_BLOCKING );

	context->WakeFunction( context );
}

/*
 * Locks should call this function to update the state machine to reflect
 * that the thread did not acquire the lock.
 */
union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical() );

	context = LockingGetContext(context);

	ASSERT( context->State == LOCKING_STATE_READY );

	//assign the blocking info so they know why they are blocked
	context->BlockingContext = * blockingInfo;
	//call the context's blocking function so the caller gets stopped approperately.
	context->BlockFunction( context );
	//set the locking context's state to blocked.
	context->State = LOCKING_STATE_BLOCKING;  

	//return link so the lock can store the blocked context.
	return &context->Link;
}

//
//End Routine
//

void LockingEnd()
{
	SchedulerEndCritical();
}

//
//State Validation Routines
//

/*
 * Threads which are trying to acquire a lock non blocking
 * should call this function to signal them that they have infact
 * acquired the lock. 
 */
BOOL LockingIsAcquired( struct LOCKING_CONTEXT * context )
{
	BOOL result;

	SchedulerStartCritical();

	ASSERT( context != NULL );

	switch( context->State )
	{
		case LOCKING_STATE_ACQUIRED:
			result = TRUE;
			context->State = LOCKING_STATE_READY;
			break;

		case LOCKING_STATE_BLOCKING:
			result = FALSE;
			break;

		case LOCKING_STATE_READY:
		default:
			KernelPanic( );
			result = FALSE;
			break;
	}

	SchedulerEndCritical();

	return result;
}

BOOL LockingIsFree( struct LOCKING_CONTEXT * context )
{
	if( context->State == LOCKING_STATE_READY )
		return TRUE;
	else
		return FALSE;
}

//
//Wake and Block functions for NonBlocking Consumers
//

void LockingWakeNonBlocking( struct LOCKING_CONTEXT * context )
{
	switch( context->State )
	{
		case LOCKING_STATE_READY:
			//we acquired the lock right away. 
			//mark as acquried.
		case LOCKING_STATE_BLOCKING:
			//we acquired the lock after blocking.
			//mark as acquired
			context->State = LOCKING_STATE_ACQUIRED;
			break;

		case LOCKING_STATE_ACQUIRED:
		default:
			KernelPanic();
			break;
	}
}

void LockingBlockNonBlocking( struct LOCKING_CONTEXT * context )
{
	return;
}

