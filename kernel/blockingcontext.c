#include"blockingcontext.h"
#include"scheduler.h"

void LockingInit( struct LOCKING_CONTEXT * context )
{
	context->State = LOCKING_STATE_READY;
}

void LockingAcquire( struct LOCKING_CONTEXT * context )
{
	if( context == NULL )
	{
		context = & SchedulerGetActiveThread()->LockingContext;
		if( context->State == LOCKING_STATE_READY || context->State == LOCKING_STATE_CHECKED )
		{
			//we acquired the lock right away, set state
			context->State = LOCKING_STATE_CHECKED;
		}
		else
		{
			KernelPanic( LOCKING_ACQUIRE_THREAD_IN_WRONG_STATE );
		}
	}
	else //context!=NULL
	{
		if( context->State == LOCKING_STATE_READY || context->State == LOCKING_STATE_CHECKED )
		{
			//we acquired lock right away, send notification
			context->State = LOCKING_STATE_ACQUIRED;
		}
		else if( context->State == LOCKING_STATE_WAITING )
		{
			//we didn't acquire right away, but now we have it.
			//send notification
			context->State = LOCKING_STATE_ACQUIRED;
		}
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

union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * context )
{
	if( context == NULL )
	{
		context = & SchedulerGetActiveThread()->LockingContext;
		context->State = LOCKING_STATE_BLOCKING;
		struct THREAD * thread = BASE_OBJECT( context, struct THREAD, LockingContext );
		ASSERT( thread == SchedulerGetActiveThread(),
				LOCKING_BLOCK_WRONG_CONTEXT,
				"the context must be owned by active thread");
		SchedulerBlockThread( );
	}
	else
	{
		context->State = LOCKING_STATE_WAITING;
	}
	return &context->Link;
}

BOOL LockingIsAcquired( struct LOCKING_CONTEXT * context )
{
	if( context->State == LOCKING_STATE_ACQUIRED )
	{
		context->State = LOCKING_STATE_CHECKED;
		return TRUE;
	}
	else if( context->State == LOCKING_STATE_WAITING )
	{
		return FALSE;
	}
	else
	{
		KernelPanic( LOCKING_IS_ACQUIRED_CONTEXT_IN_WRONG_STATE );
	}
}

