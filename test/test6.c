#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/worker.h"
#include"../kernel/critinterrupt.h"
#include"../kernel/semaphore.h"
#include"../kernel/panic.h"
#include"../kernel/timer.h"

#define STACK_SIZE HAL_MIN_STACK_SIZE

//Context for work item.

struct WORKER_CONTEXT 
{
	COUNT Count;
};

//Define structures

char WorkerStack[STACK_SIZE];
char WorkerStack[STACK_SIZE];

struct WORKER_QUEUE WorkerQueue;

struct SEMAPHORE Semaphore;

struct WORKER_ITEM ProducerItem;
struct WORKER_ITEM ConsumerItem;

struct WORKER_CONTEXT ProducerContext;
struct WORKER_CONTEXT ConsumerContext;

struct HANDLER_OBJECT WorkTimer;

//Prototypes

BOOL WorkTimerHandler( struct HANDLER_OBJECT * timer );
enum WORKER_RETURN WorkerConsumerTask( struct WORKER_ITEM * item );
enum WORKER_RETURN WorkerProducerTask( struct WORKER_ITEM * item );
BOOL WorkCritHandler( struct HANDLER_OBJECT * timer );

//Handlers

BOOL WorkTimerHandler( struct HANDLER_OBJECT * timer ) 
{
	CritInterruptRegisterHandler(
			timer, 
			WorkCritHandler,
			NULL );

	return FALSE;
}

BOOL WorkCritHandler( struct HANDLER_OBJECT * timer )
{
	//TODO WE NEED TO FIX WORKER UNIT TO ACT LIKE HANDLERS.
	static BOOL FirstPass = TRUE;

	if( WorkerItemIsFinished(&ProducerItem) || FirstPass )
	{
		WorkerInitItem( &WorkerQueue, WorkerProducerTask, &ProducerContext, &ProducerItem );
	}

	if( FirstPass )
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

enum WORKER_RETURN WorkerConsumerTask( struct WORKER_ITEM * item )
{
	enum WORKER_RETURN returnValue;
	struct WORKER_CONTEXT * workContext = WorkerGetContext( item );
	struct LOCKING_CONTEXT * lockContext = WorkerGetLockingContext( item );

	do 
	{
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
			returnValue = WORKER_FINISHED;
		}
		else
		{
			//the lock is not acquired, so lets block the work item.
			returnValue = WORKER_BLOCKED; 
		}

		//We repeat downing the semaphore so that we drain it.
	}while(returnValue != WORKER_BLOCKED);

	return returnValue;
}

enum WORKER_RETURN WorkerProducerTask( struct WORKER_ITEM * item )
{
        struct WORKER_CONTEXT * workContext = WorkerGetContext( item );
        SemaphoreUp( &Semaphore );
        workContext->Count++;
        return WORKER_FINISHED;
}

//TODO We should have a normal thread fire off work items as well to test 
//the thread paths.

//Main

int main()
{
        KernelInit();

        SchedulerStartup();

        WorkerCreateWorker(
                        &WorkerQueue,
                        WorkerStack,
                        STACK_SIZE);

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

