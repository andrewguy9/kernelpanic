#include"worker.h"
#include"../utils/linkedlist.h"
#include"interrupt.h"
#include"scheduler.h"

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
	return;
}

void WorkerWakeOnLock( struct LOCKING_CONTEXT * context )
{
	struct WORK_ITEM * worker;

	worker = BASE_OBJECT( context,
				struct WORKER_ITEM,
				LockingContext);

	//Now that the work item is unblocked, we can
	//add it back to the work item queue.
	AddItem( worker );
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
	
	while(TRUE)
	{
		//Fetch a handler
		item = GetItem();

		if( item == NULL )
		{//there is no item, lets switch threads
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
		else
		{//there is a item, so execute it.

			//run handler
			result = item->Foo(item);

			//Determine what to do with work item.
			InterruptDisable();
			if( result == WORKER_FINISHED )
			{
				//the item is done, mark so caller knows.
				item->Finished = TRUE;
			}
			else if( result == WORKER_PENDED )
			{
				//the item needs more processing. 
				//add back into queue.
				LinkedListEnqueue( &item->Link.LinkedListLink, &WorkerItemQueue );
			}
			else if( result == WORKER_BLOCKED )
			{
				//the work item is blocked on a lock, do
				//nothing.
			}
			InterruptEnable();
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
	item->Finished = FALSE;
	item->Context = context;

	LinkedListEnqueue( &item->Link.LinkedListLink, &WorkerItemQueue );
	
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
