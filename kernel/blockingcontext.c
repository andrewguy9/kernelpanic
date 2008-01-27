#include"blockingcontext.h"
#include"scheduler.h"

void LockingInit( struct LOCKING_CONTEXT * context )
{
	context->State = LOCKING_STATE_READY;
}

void LockingAcquire( struct LOCKING_CONTEXT *context )
{
	if( context == NULL )
	{
		SchedulerGetActiveThread()->LockingContext.State = LOCKING_STATE_CHECKED;
	}
	else
	{
		context->State = LOCKING_STATE_CHECKED;
	}
}

void LockingUnblock( struct LOCKING_CONTEXT *context)
{
	if( context->State == LOCKING_STATE_BLOCKING )
	{
		context->State = LOCKING_STATE_CHECKED;
	}
	else if( context->State == LOCKING_STATE_WAITING )
	{
		context->State = LOCKING_STATE_ACQUIRED;
	}
	else
	{
		KernelPanic( LOCKING_BLOCK_INVALID_CONTEXT );
	}
}

union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * waitingContext )
{
	struct LOCKING_CONTEXT * context;
	if( waitingContext == NULL )
	{
		context = & SchedulerGetActiveThread()->LockingContext;
	}
	else
	{
		context = waitingContext;
	}

	ASSERT( context->State != LOCKING_STATE_BLOCKING && 
			context->State != LOCKING_STATE_WAITING && 
			context->State != LOCKING_STATE_ACQUIRED,
			LOCKING_BLOCK_INVALID_CONTEXT,
			"must have context in ready or checked state");

	if( waitingContext == NULL )
	{
		context->State = LOCKING_STATE_BLOCKING;
	}
	else
	{
		context->State = LOCKING_STATE_WAITING;
	}

	if( blockingInfo != NULL )
		context->BlockingContext = * blockingInfo;

	return &context->Link;
}

BOOL LockingIsAcquired( struct LOCKING_CONTEXT * context )
{
	if( context->State == LOCKING_STATE_ACQUIRED || context->State == LOCKING_STATE_CHECKED )
	{
		context->State = LOCKING_STATE_CHECKED;
		return TRUE;
	}
	else
	{
		ASSERT( context->State != LOCKING_STATE_READY,
				LOCKING_IS_ACQUIRED_INVALID_CONTEXT,
				"Cant use unblocked context");
		return FALSE;
	}
}

