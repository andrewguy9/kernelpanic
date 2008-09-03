#include"worker.h"
#include"../utils/linkedlist.h"
#include"interrupt.h"
#include"scheduler.h"
#include"panic.h"

struct LINKED_LIST WorkerItemQueue;

//
//Helper Functions
//

struct WORKER_ITEM * WorkerGetItem()
{
	struct LINKED_LIST_LINK * link;

	link = LinkedListPop( &WorkerItemQueue );

	if( link == NULL )
		return NULL;
	else
		return BASE_OBJECT( link, struct WORKER_ITEM, Link );
}

void AddItem( struct WORKER_ITEM * worker )
{
	InterruptDisable();
	LinkedListEnqueue( &worker->Link.LinkedListLink, &WorkerItemQueue );
	InterruptEnable();
}

//
//Locking block and wake functions
//

void WorkerBlockOnLock( struct LOCKING_CONTEXT * context )
{
	//We do nothing because the work item is automatically
	//pulled out of the work queue, and is added to the lock list.
	return;//TODO IS THIS CORRECT.
}

void WorkerWakeOnLock( struct LOCKING_CONTEXT * context )
{
	struct WORKER_ITEM * worker;

	worker = BASE_OBJECT( context,
				struct WORKER_ITEM,
				LockingContext);

	switch( context->State )
	{
		case LOCKING_STATE_READY:
			//we acquired the lock right away.
			//mark is acquired.
			context->State = LOCKING_STATE_ACQUIRED;
			break;

		case LOCKING_STATE_BLOCKING:
			//We acquired the lock after blocking.
			//mark as acquired and add to work queue.
			AddItem(worker);
			context->State = LOCKING_STATE_ACQUIRED;
			break;

		case LOCKING_STATE_ACQUIRED:
		default:
			KernelPanic();
			break;
	}
}

//
//Public Functions
//

void WorkerStartup()
{
	LinkedListInit( & WorkerItemQueue );	
}

void WorkerThreadMain()
{
	struct WORKER_ITEM * item;
	enum WORKER_RETURN result;
	
	while(TRUE)
	{
		//Fetch a handler
		item = WorkerGetItem();

		if( item == NULL )
		{
			//there is no item, lets switch threads
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
		else
		{
			ASSERT(! item->Finished);
			//there is a item, so execute it.
			result = item->Foo(item);

			//Determine what to do with work item.
			switch( result )
			{
				case WORKER_FINISHED:
					//the item is done, mark so caller knows.
					item->Finished = TRUE;
					break;

				case WORKER_BLOCKED:
					//the work item is blocked on a lock, the lock will store 
					//a link to the item. do nothing.
					break;

				case WORKER_PENDED:
				//the item needs more processing. 
				//add back into queue.
				LinkedListEnqueue( &item->Link.LinkedListLink, &WorkerItemQueue );
				break;
			}
		}
	}
}

void WorkerCreateWorker(
		struct THREAD * thread,
		char * stack,
		unsigned int stackSize,
		char flag)
{
	SchedulerCreateThread( 
			thread,
			10,
			stack,
			stackSize,
			WorkerThreadMain,
			NULL,
			flag,
			TRUE);
}

void WorkerInitItem( WORKER_FUNCTION foo, void * context, struct WORKER_ITEM * item  )
{
	InterruptDisable();

	item->Foo = foo;
	LockingInit( &item->LockingContext, WorkerBlockOnLock, WorkerWakeOnLock );
	item->Context = context;

	LinkedListEnqueue( &item->Link.LinkedListLink, &WorkerItemQueue );
	item->Finished = FALSE;
	InterruptEnable();
}

BOOL WorkerItemIsFinished( struct WORKER_ITEM * item )
{
	BOOL result;
	InterruptDisable();
	result = item->Finished;
	InterruptEnable();
	return result;
}

void * WorkerGetContext( struct WORKER_ITEM * item )
{
	return item->Context;
}

struct LOCKING_CONTEXT * WorkerGetLockingContext( struct WORKER_ITEM * item )
{
	ASSERT( item!=NULL );
	return &item->LockingContext;
}
