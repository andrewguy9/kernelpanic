#include"worker.h"
#include"../utils/linkedlist.h"
#include"interrupt.h"
#include"softinterrupt.h"
#include"scheduler.h"
#include"panic.h"

/*
 * Worker Unit:
 * The worker unit allows for threads or ISRs to
 * queue work items. 
 *
 * TODO: See if we can move from enable/disable interrupt to crit.
 */

//
//Helper Functions
//

struct WORKER_ITEM * WorkerGetItem( struct WORKER_QUEUE * queue )
{
	struct LINKED_LIST_LINK * link;

	//TODO THIS SHOULD BE SAFE BECAUSE IT WILL BE CALLED
	//IN A THREAD'S CONTEXT.
	SemaphoreDown( & queue->Lock, NULL );

	InterruptDisable();
	link = LinkedListPop( & queue->List );
	InterruptEnable();

	ASSERT( link != NULL );
	return BASE_OBJECT( link, struct WORKER_ITEM, Link );
}

/*
 * Adds a work item 'item' to the work queue 'queue'.
 * It is not safe to add work items to the queue which have already been 
 * registered. Calling WorkerItemIsFinished() can solve this engima.
 */
void WorkerAddItem( struct WORKER_QUEUE * queue, struct WORKER_ITEM * item )
{

	//TODO THIS MAY NOT BE SAFE BECAUSE IT WILL BE CALLED FROM 
	//INSIDE CRITICAL SECTIONS.
	
	//Make sure we are at thread level.
	ASSERT( !InterruptIsAtomic() && !SoftInterruptIsAtomic() );

	SemaphoreUp( & queue->Lock );

	//We mark finished inside interrupt section so that WorkerItemIsFinished
	//returns accurate results.
	InterruptDisable();
	item->Finished = FALSE;
	item->Queue = queue;
	LinkedListEnqueue( &item->Link.LinkedListLink, & queue->List );
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
	struct WORKER_ITEM * worker;
	struct WORKER_QUEUE * queue;

	worker = BASE_OBJECT( context,
				struct WORKER_ITEM,
				LockingContext);
	queue = worker->Queue;

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
			WorkerAddItem( queue, worker );
			context->State = LOCKING_STATE_ACQUIRED;
			break;

		case LOCKING_STATE_ACQUIRED:
		default:
			KernelPanic();
			break;
	}
}

//
//Thread Main for worker threads
//

void WorkerThreadMain( void * arg )
{
	struct WORKER_QUEUE * queue;
	struct THREAD * thread;
	struct SEMAPHORE * lock;
	struct LINKED_LIST * list;

	struct WORKER_ITEM * item;
	enum WORKER_RETURN result;

	//Get the Work Thread Context
	queue = arg;
	thread = & queue->Thread;
	lock = & queue->Lock;
	list = & queue->List;
	
	//Loop consuming work items. 
	while(TRUE)
	{
		//Fetch a handler
		item = WorkerGetItem( queue );

		ASSERT(! item->Finished );
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
				//the work item is blocked on a lock, the lock will 
				//re-queue the work item when the lock has been acquired.
				break;

			case WORKER_PENDED:
				//the item needs more processing. 
				//add back into queue.
				LinkedListEnqueue( &item->Link.LinkedListLink, list );
				break;
		}// end switch
	}// end while(TRUE).
}

//
//Public Functions
//

void WorkerCreateWorker(
		struct WORKER_QUEUE * queue,
		char * stack,
		unsigned int stackSize,
		INDEX flag)
{
	SchedulerCreateThread( 
			&queue->Thread,
			10,
			stack,
			stackSize,
			WorkerThreadMain,
			queue,
			flag,
			TRUE);
}

void WorkerInitItem( struct WORKER_QUEUE * queue, WORKER_FUNCTION foo, void * context, struct WORKER_ITEM * item  )
{
	//Its assumed that the caller of WorkerInitItem is the sole owner of the item
	//at the time it is called. This means we dont have to disable interrupts around
	//the init code.
	
	item->Foo = foo;
	LockingInit( &item->LockingContext, WorkerBlockOnLock, WorkerWakeOnLock );
	item->Context = context;


	//Now we are going to insert the work item into the list.
	WorkerAddItem(queue, item );
}

/*
 * WorkerItemIsFinished should be used by creator of a work item
 * to see if his work item structure is free for use. If 
 * WorkerItemIsFinished returns TRUE, then it is safe to use
 * the work item.
 *
 * If it returns FALSE, then the work item is still either
 * queued, or currently running. 
 *
 * USAGE:
 * Only the work item's owner should call WorkerItemIsFinished. 
 * It is the caller's responsibility to avoid racing to reclaim 
 * work items.
 */
BOOL WorkerItemIsFinished( struct WORKER_ITEM * item )
{
	BOOL result;
	InterruptDisable();
	result = item->Finished;
	InterruptEnable();
	return result;
}

/*
 * Returns the work item's context field. This is provided by the
 * owner of the work item.
 */
void * WorkerGetContext( struct WORKER_ITEM * item )
{
	return item->Context;
}

/*
 * when acquiring a lock in a work item you must use the work item's 
 * locking context (dont use NULL!). This is because blocking the 
 * worker thread may starve another work item which holds the lock
 * you are waiting on. To solve this the work item structure
 * contains a locking context so that every work item can just use the 
 * provided context. To get access to the context first call
 * Worker GetLockingContext. Once you have a pointer to the context, 
 * acquire the lock with it. (Lock(somelock, workerlockingcontext)).
 * Use LockingIsAcquired to determine if you actually acquired the lock
 * or if you are waiting. If you are waiting, you can return WORKER_BLOCKED
 * to wait until you own the lock.
 * 
 */
struct LOCKING_CONTEXT * WorkerGetLockingContext( struct WORKER_ITEM * item )
{
	ASSERT( item!=NULL );
	return &item->LockingContext;
}
