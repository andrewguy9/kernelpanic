#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/worker.h"
#include"../kernel/interrupt.h"
#include"../kernel/semaphore.h"

//Context for work item.

struct WORKER_CONTEXT 
{
	COUNT * Count;
	struct LOCKING_CONTEXT LockingContext;
};


char WorkerStack[300];
char MainStack[300];

struct THREAD WorkerThread;
struct THREAD MainThread;

struct SEMAPHORE Semaphore;

enum WORKER_RETURN WorkerConsumerTask( struct WORKER_ITEM * item )
{
	struct WORKER_CONTEXT * context = item->Context;
	if( LockingIsAcquired( &context->LockingContext ) )
	{
		//The context is not in use, so we have not started
		//trying to acquire the lock! So lets try now.

		SemaphoreDown( &Semaphore, &context->LockingContext );
	}

	//at this point we know we have tried to acquire the lock.
	//lets see if we actually did it.
	
	if( LockingIsAcquired( &context->LockingContext ) )
	{
		// we got the lock. Lets do the work.
		context->Count++;
		//we have done the work, now lets finish the work item.
		return WORKER_FINISHED;
	}
	else
	{
		//the lock is not acquired, so lets pend the work item.
		return WORKER_PENDED; 
	}

}

enum WORKER_RETURN WorkerProducerTask( struct WORKER_ITEM * item )
{
	SemaphoreUp( &Semaphore );
	return WORKER_FINISHED;
}

struct WORKER_ITEM ProducerItem;
struct WORKER_ITEM ConsumerItem;

struct WORKER_CONTEXT ProducerContext;
struct WORKER_CONTEXT ConsumerContext;

void ThreadMain()
{
	WorkerAddItem( WorkerProducerTask, &ProducerContext, &ProducerItem );
	WorkerAddItem( WorkerConsumerTask, &ConsumerContext, &ConsumerItem );
	while(TRUE)
	{
		if( WorkerItemIsFinished(&ProducerItem) )
		{
			WorkerAddItem( WorkerProducerTask, &ProducerContext, &ProducerItem );
		}

		if( WorkerItemIsFinished(&ConsumerItem) )
		{
			WorkerAddItem( WorkerConsumerTask, &ConsumerContext, &ConsumerItem );
		}
	}
}

int main()
{
	KernelInit();

	SchedulerCreateThread( 
			&MainThread, 
			2, 
			MainStack, 
			300, 
			ThreadMain, 
			NULL,
			0x01 , 
			TRUE );

	WorkerCreateWorker(
			&WorkerThread,
			WorkerStack,
			300,
			0x02 );

	ProducerContext.Count = 0; 
	LockingInit( &ProducerContext.LockingContext, NULL, NULL );//TODO

	ConsumerContext.Count = 0;
	LockingInit( &ConsumerContext.LockingContext, NULL, NULL );//TODO

	KernelStart();
	return 0;
}
