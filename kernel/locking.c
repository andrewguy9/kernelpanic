#include"locking.h"
#include"scheduler.h"
#include"panic.h"

/*
 * Unit Description
 *
 * Locking unit provides a locking state machine which
 * aids in implementing locks.
 *
 * The state machine makes the appropriate callbacks
 * to block/pass/pend on a lock. Various components
 * implement callbacks for the locking unit to call.
 *
 * 1) Scheduler - When a thread blocks on a lock,
 *                the scheduler's locking callbacks
 *                block the thread on the lock.
 *
 * 2) Locking Unit - When a lock is locked using the
 *                   locking unit callbacks, the 
 *                   caller should call LockingIsAcquired
 *                   to determine if they did manage to 
 *                   get the lock.
 *
 * 3) Worker Unit - When a lock is locked in a work item,
 *                  the caller should use the worker callbacks.
 *                  They should use LockingIsAcquired to determine
 *                  if they acquired the lock. If they didn't
 *                  then they should return. The worker unit's 
 *                  locking callbacks will re-queue the work item
 *                  when the lock is acquired.
 *
 * This scheme means that we never have people poll on a lock. 
 * In some systems people try to acquire locks non-blocking, and
 * don't register a notification method when they are granted the 
 * lock. Instead they say, "give me the lock if its unlocked, other
 * wise just forget I asked". This is not a good idea because locks
 * under contention may never be granted to the poller. The locking
 * unit always requires that a locking context be used in order to
 * acquire a lock. 
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
	if( blockingInfo != NULL )
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

