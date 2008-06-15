#include"locking.h"
#include"scheduler.h"

/*
 * Unit Description
 *
 * Locking unit provides a locking state machine which
 * can be used to various locks for abstractify calls
 * to the scheduler.
 *
 * Using this statemachine, locks can be implemented 
 * to be both blocking or nonblocking with minimal effort.
 */

/*
 * Locks should call LockingStart to start a critical section.
 */
void LockingStart()
{
	ASSERT( !SchedulerIsCritical() );

	SchedulerStartCritical();
}

/*
 * Locks should call LockingEnd to end critical sections
 * on unlock() operations.
 */
void LockingEnd(  )
{
	ASSERT( SchedulerIsCritical() );

	SchedulerEndCritical();
}

/*
 * Locks should call LockingSwitch to end a critical section and 
 * context switch if needed. This is used in lock() operations. 
 */
void LockingSwitch( struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical() );

	if( context == NULL )
	{
		//There is no context, so the thread must be on blocking path.
		context = SchedulerGetLockingContext();

		ASSERT( context->State != LOCKING_STATE_WAITING );
	}
	else
	{
		//There is a context, so thread must be on waiting path.
		ASSERT( context->State != LOCKING_STATE_BLOCKING );
	}
	
	switch( context->State )
	{
		case LOCKING_STATE_BLOCKING:
			//we are to block on the lock, switch out
			SchedulerForceSwitch();
			break;

		case LOCKING_STATE_WAITING:
			//we are to wait on the lock, continue in slice
			SchedulerEndCritical();
			break;

		case LOCKING_STATE_CHECKED:
		case LOCKING_STATE_ACQUIRED:
			//we acquired the lock, continue
			SchedulerEndCritical();
			break;
		case LOCKING_STATE_READY:
			//lock is marked as ready, this is illegal
			KernelPanic( );
	}
}

/*
 * Initialize a locking context for use.
 * NOT ATOMIC
 */
void LockingInit( struct LOCKING_CONTEXT * context )
{
	context->State = LOCKING_STATE_READY;
}

/*
 * Locks should call LockingAcquire to update the statemachine to
 * reflect that the thread has taken the lock. This call should be 
 * surrounded by LockingStart() and LockingEnd/LockingSwitch
 */
void LockingAcquire( struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical() );

	if( context == NULL )
	{
		//We must retrieve the context from the active thread.
		context = SchedulerGetLockingContext();

		//If the context is null, then we:
		//locked successfully without blocking : state == ready or 
		//checked -> state == checked
		if( context->State == LOCKING_STATE_READY || 
				context->State == LOCKING_STATE_CHECKED )
		{
			context->State = LOCKING_STATE_CHECKED;
		}
		else
		{
			KernelPanic( );
		}
	}
	else //context!=NULL
	{
		//if we were provided a context
		//then we are in:
		//locked successfully without waiting : state == ready or 
		//checked -> state == acquired
		if( context->State == LOCKING_STATE_READY ||
				context->State == LOCKING_STATE_CHECKED )
		{
			//we acquired lock right away, send notification
			context->State = LOCKING_STATE_ACQUIRED;
		}
		//locked successfully after waiting -> state == waiting -> state == acquired
		else if( context->State == LOCKING_STATE_WAITING )
		{
			//The thread was waiting, mark as acquired
			context->State = LOCKING_STATE_ACQUIRED;
		}
		//locked successfully with blocking -> state == blocking
		else if( context->State == LOCKING_STATE_BLOCKING )
		{
			//the thread got blocked. we need to wake him.
			struct THREAD *thread = BASE_OBJECT(context, struct THREAD, LockingContext);
			SchedulerResumeThread( thread );
			context->State = LOCKING_STATE_CHECKED;
		}

		else
		{
			KernelPanic( );
		}
	}
}

/*
 * Locks should call this function to update the state machine to reflect
 * that the thread did not acquire the lock.
 */
union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical() );

	if( context == NULL )
	{
		//context is active thread, so block him.
		context = SchedulerGetLockingContext();
		context->State = LOCKING_STATE_BLOCKING;
		SchedulerBlockThread( );
		
	}
	else
	{//context is user specified, make him wait.
		context->State = LOCKING_STATE_WAITING;
	}

	//assign the blocking info so they know why they are blocked
	context->BlockingContext = * blockingInfo;

	//return link so they can store blocked thread.
	
	ASSERT( context != NULL );

	return &context->Link;
}

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
			context->State = LOCKING_STATE_CHECKED;
			result = TRUE;
			break;

		case LOCKING_STATE_WAITING:
			result = FALSE;
			break;

		case LOCKING_STATE_READY:
		case LOCKING_STATE_BLOCKING:
		case LOCKING_STATE_CHECKED:
			KernelPanic( );
			result = FALSE;
			break;
	}

	SchedulerEndCritical();
	return result;
}

