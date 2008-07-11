#include"locking.h"
#include"scheduler.h"

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

	switch( context->State )
	{
		case LOCKING_STATE_READY:
			//we acquired the lock right away. mark.
			context->State = LOCKING_STATE_ACQUIRED;
			break;

		case LOCKING_STATE_BLOCKING:
			//We acquired the lock after blocking.
			//We need to wake the caller.
			context->WakeFunction( context );
			context->State = LOCKING_STATE_READY;
			break;

		case LOCKING_STATE_ACQUIRED:
		default:
			KernelPanic();
	}
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
	//set the state of the context
	context->State = LOCKING_STATE_BLOCKING;

	//return link so the lock can store the blocked context.
	return &context->Link;
}

//
//End Routines
//

/*
 * Locks should call LockingSwitch to end a critical section and 
 * context switch if needed. 
 */
void LockingSwitch( struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical() );

	context = LockingGetContext( context );
	
	switch( context->State )
	{
		case LOCKING_STATE_READY:
			//When this routine is called the lock must have specified 
			//weather its acquired or blocked.
			KernelPanic();
			break;

		case LOCKING_STATE_BLOCKING:
			//This caller is going to be blocked, continue.
			break;

		case LOCKING_STATE_ACQUIRED:
			//The lock said that this context was granted the lock.
			//We'll mark the context as ready to acnolege this and retrun.
			context->State = LOCKING_STATE_READY;
			break;
	}

	//The lock calling switch is the singal to end the locking operation.
	//Now we end the critical section and switch threads (if needed).
	SchedulerEndCritical();
}

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
		case LOCKING_STATE_READY:
			result = TRUE;
			break;

		case LOCKING_STATE_BLOCKING:
			result = FALSE;
			break;

		case LOCKING_STATE_ACQUIRED:
		default:
			KernelPanic( );
			result = FALSE;
			break;
	}

	SchedulerEndCritical();

	return result;
}
