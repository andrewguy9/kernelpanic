#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/worker.h"
#include"../kernel/interrupt.h"
#include"../kernel/semaphore.h"
#include"../kernel/panic.h"
#include"../kernel/timer.h"

//Context for work item.

struct WORKER_CONTEXT 
{
	COUNT * Count;
};

#define STACK_SIZE HAL_MIN_STACK_SIZE

//
//Validate State
//

char WorkerStack[STACK_SIZE];

struct WORKER_QUEUE WorkerQueue;

struct SEMAPHORE Semaphore;

enum WORKER_RETURN WorkerConsumerTask( struct WORKER_ITEM * item )
{
	struct WORKER_CONTEXT * workContext = WorkerGetContext( item );
	struct LOCKING_CONTEXT * lockContext = WorkerGetLockingContext( item );

	if( LockingIsFree( lockContext ) )
	{
		//The context is not in use, so we have not started
		//trying to acquire the lock! So lets try now.
		SemaphoreDown( &Semaphore, lockContext );
	}

	//at this point we know we have tried to acquire the lock.
	//lets see if we actually did it.
	
	if( LockingIsAcquired( lockContext ) )
	{
		// we got the lock. Lets do the work.
		workContext->Count++;
		//we have done the work, now lets finish the work item.
		return WORKER_FINISHED;
	}
	else
	{
		//the lock is not acquired, so lets block the work item.
		return WORKER_BLOCKED; 
	}
}

enum WORKER_RETURN WorkerProducerTask( struct WORKER_ITEM * item )
{
	struct WORKER_CONTEXT * workContext = WorkerGetContext( item );
	SemaphoreUp( &Semaphore );
	workContext->Count++;
	return WORKER_FINISHED;
}

struct WORKER_ITEM ProducerItem;
struct WORKER_ITEM ConsumerItem;

struct WORKER_CONTEXT ProducerContext;
struct WORKER_CONTEXT ConsumerContext;

struct HANDLER_OBJECT WorkTimer;
BOOL WorkTimerHandler( struct HANDLER_OBJECT * timer ) 
{
	//TODO WE NEED TO FIX WORKER UNIT TO ACT LIKE HANDLERS.
	static BOOL FirstPass = TRUE;

	if( WorkerItemIsFinished(&ProducerItem) || FirstPass )
	{
		WorkerInitItem( &WorkerQueue, WorkerProducerTask, &ProducerContext, &ProducerItem );
	}
	
	if( WorkerItemIsFinished(&ConsumerItem) || FirstPass )
	{
		WorkerInitItem( &WorkerQueue, WorkerConsumerTask, &ConsumerContext, &ConsumerItem );
	}

	TimerRegister(
			timer,
			1,
			WorkTimerHandler,
			NULL );

	FirstPass = FALSE;
	return FALSE;
}

int main()
{
	KernelInit();

	WorkerCreateWorker(
			&WorkerQueue,
			WorkerStack,
			STACK_SIZE,
			1 );

	ProducerContext.Count = 0; 
	ConsumerContext.Count = 0;

	HandlerInit( &WorkTimer );
	TimerRegister(
			&WorkTimer,
			1,
			WorkTimerHandler,
			NULL );

	KernelStart();

	return 0;
}

