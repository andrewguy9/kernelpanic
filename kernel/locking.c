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
	ASSERT( !SchedulerIsCritical(),
			LOCKING_START_NOT_CRIT,
			"We can only start a locking session when not critical");

	SchedulerStartCritical();
}

/*
 * Locks should call LockingEnd to end critical sections
 * on unlock() operations.
 */
void LockingEnd(  )
{
	ASSERT( SchedulerIsCritical(),
		 LOCKING_END_NOT_CRIT,
		"If we are ending a locking session we must be critical already" );

	SchedulerEndCritical();
}

/*
 * Locks should call LockingSwitch to end a critical section and 
 * context switch if needed. This is used in lock() operations. 
 */
void LockingSwitch( struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical(),
			LOCKING_SWITCH_NOT_CRIT,
			"must be crit to switch");

	if( context == NULL )
	{
		//There is no context, so the thread must be on blocking path.
		context = & SchedulerGetActiveThread()->LockingContext;

		ASSERT( context->State != LOCKING_STATE_WAITING,
			 LOCKING_SWITCH_NULL_WAITING,
			"if the context is null, its implied the thread will block, not wait" );
	}
	else
	{
		//There is a context, so thread must be on waiting path.
		ASSERT( context->State != LOCKING_STATE_BLOCKING,
			 LOCKING_SWITCH_CONTEXT_BLOCKING,
			"when a context is provided its assumed we will not block" );
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
			KernelPanic( LOCKING_SWITCH_CONTEXT_READY );
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
	ASSERT( SchedulerIsCritical(),
			LOCKING_ACQUIRE_NOT_CRIT,
			"must be crit to acquire");

	if( context == NULL )
	{
		//We must retrieve the context from the active thread.
		context = & SchedulerGetActiveThread()->LockingContext;

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
			KernelPanic( LOCKING_ACQUIRE_THREAD_IN_WRONG_STATE );
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
			KernelPanic( LOCKING_ACQUIRE_CONTEXT_IN_WRONG_STATE );
		}
	}
}

/*
 * Locks should call this function to update the state machine to reflect
 * that the thread did not acquire the lock.
 */
union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * context )
{
	ASSERT( SchedulerIsCritical(),
			LOCKING_BLOCK_IS_CRIT,
			"Must be crit to block");

	if( context == NULL )
	{
		//context is active thread, so block him.
		context = & SchedulerGetActiveThread()->LockingContext;
		context->State = LOCKING_STATE_BLOCKING;
		struct THREAD * thread = BASE_OBJECT( 
				context, 
				struct THREAD, 
				LockingContext );
		ASSERT( thread == SchedulerGetActiveThread(),
				LOCKING_BLOCK_WRONG_CONTEXT,
				"the context must be owned by active thread" );
		SchedulerBlockThread( );
		
	}
	else
	{//context is user specified, make him wait.
		context->State = LOCKING_STATE_WAITING;
	}

	//assign the blocking info so they know why they are blocked
	context->BlockingContext = * blockingInfo;

	//return link so they can store blocked thread.
	
	ASSERT( context != NULL, 
			LOCKING_BLOCK_CONTEXT_NOT_NULL,
			"we have to return a link, so context cant be null");

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

	ASSERT( context != NULL, 
			LOCKING_IS_ACQUIRED_CONTEXT_NULL,
		   	"context must not be null" );

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
			KernelPanic( LOCKING_IS_ACQUIRED_CONTEXT_IN_WRONG_STATE );
			result = FALSE;
			break;
	}

	SchedulerEndCritical();
	return result;
}

