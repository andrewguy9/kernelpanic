#include"blockingcontext.h"
#include"scheduler.h"

void LockingInit( struct LOCKING_CONTEXT * context )
{
	context->State = LOCKING_STATE_READY;
}

union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo )
{
	struct LOCKING_CONTEXT * context = & SchedulerGetActiveThread()->LockingContext;

	if( context->State != LOCKING_STATE_READY && 
			context->State != LOCKING_STATE_ACQUIRED )
		KernelPanic( LOCKING_BLOCK_WRONG_STATE );

	context->State = LOCKING_STATE_BLOCKING;
	
	if( blockingInfo != NULL )
		context->BlockingContext = * blockingInfo;

	SchedulerBlockThread();
	return &context->Link;
}

union LINK * LockingWait( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * context )
{

	if( context->State != LOCKING_STATE_READY && 
			context->State != LOCKING_STATE_ACQUIRED )
		KernelPanic( LOCKING_WAIT_WRONG_STATE );

	context->State = LOCKING_STATE_WAITING;

	if( blockingInfo != NULL )
		context->BlockingContext = * blockingInfo;

	return &context->Link;
}

BOOL LockingIsAcquired( struct LOCKING_CONTEXT * context )
{
	return context->State == LOCKING_STATE_ACQUIRED;
}

union LINK * LockingAcquire( struct LOCKING_CONTEXT * context )
{
	if(context->State == LOCKING_STATE_BLOCKING)
	{
		//Thread is blocking, so wake him.
		struct THREAD * thread = BASE_OBJECT( context, struct THREAD, LockingContext );
		SchedulerResumeThread( thread );
	}
	else if( context->State == LOCKING_STATE_WAITING )
	{
		//We dont have to do anything if the thread is waiting
	}
	else
	{
		//These are illegal states to call acquire for!
		KernelPanic( LOCKING_ACQUIRE_WRONG_STATE );
	}

	context->State = LOCKING_STATE_ACQUIRED;
}

