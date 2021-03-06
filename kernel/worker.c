#include"worker.h"
#include"utils/linkedlist.h"
#include"isr.h"
#include"softinterrupt.h"
#include"critinterrupt.h"
#include"scheduler.h"
#include"panic.h"

/*
 * Worker Unit:
 * The worker unit allows for threads or ISRs to
 * queue work items.
 */

/*
 * NOTE: Work Item Queues are protected at IRQ_LEVEL_CRIT.
 * That means that only thread or CritInterrupts may register
 * new work items!
 */

//
//Helper Functions
//

struct WORKER_ITEM * WorkerGetItem( struct WORKER_QUEUE * queue )
{
        struct LINKED_LIST_LINK * link;

        //Note: This is safe because WorkerGetItem is only called in a thread
        //context.
        SemaphoreDown( & queue->Lock, NULL );

        CritInterruptDisable();
        link = LinkedListPop( & queue->List );
        CritInterruptEnable();

        ASSERT( link != NULL );
        return container_of( link, struct WORKER_ITEM, Link );
}

/*
 * Adds a work item 'item' to the work queue 'queue'.
 * It is not safe to add work items to the queue which have already been
 * registered. Calling WorkerItemIsFinished() can solve this engima.
 */
void WorkerAddItem( struct WORKER_QUEUE * queue, struct WORKER_ITEM * item )
{

        //Make sure we are at thread level.
        ASSERT( !SoftInterruptIsAtomic() );

        SemaphoreUp( & queue->Lock );

        //We mark finished inside interrupt section so that WorkerItemIsFinished
        //returns accurate results.
        item->Queue = queue;

        CritInterruptDisable();
        //Finished needs to be updated with critical sections disabled.
        item->Finished = false;
        //Adding to the queue needs to be done with Critinterrupts Disabled.
        LinkedListEnqueue( &item->Link.LinkedListLink, & queue->List );
        CritInterruptEnable();
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

        worker = container_of( context,
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

THREAD_MAIN WorkerThreadMain;
void * WorkerThreadMain( void * arg )
{
        struct WORKER_QUEUE * queue;
        struct LINKED_LIST * list;

        struct WORKER_ITEM * item;
        enum WORKER_RETURN result;

        //Get the Work Thread Context
        queue = arg;
        list = & queue->List;

        //Loop consuming work items.
        while(true) {
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
                                item->Finished = true;
                                break;

                        case WORKER_BLOCKED:
                                //the work item is blocked on a lock, the lock will
                                //re-queue the work item when the lock has been acquired.
                                break;

                        case WORKER_PENDED:
                                //the item needs more processing.
                                //add back into queue.
                                CritInterruptDisable();
                                LinkedListEnqueue( &item->Link.LinkedListLink, list );
                                CritInterruptEnable();
                                break;
                }// end switch
        }// end while(true).
        return NULL;
}

//
//Public Functions
//

void WorkerCreateWorker(
                struct WORKER_QUEUE * queue,
                char * stack,
                unsigned int stackSize)
{
        LinkedListInit( &queue->List );

        SemaphoreInit( &queue->Lock, 0 );

        SchedulerCreateThread(
                        &queue->Thread,
                        10,
                        stack,
                        stackSize,
                        WorkerThreadMain,
                        queue,
                        NULL,
                        true);
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
 * WorkerItemIsFinished returns true, then it is safe to use
 * the work item.
 *
 * If it returns false, then the work item is still either
 * queued, or currently running.
 *
 * USAGE:
 * Only the work item's owner should call WorkerItemIsFinished.
 * It is the caller's responsibility to avoid racing to reclaim
 * work items.
 */
_Bool WorkerItemIsFinished( struct WORKER_ITEM * item )
{
        _Bool result;
        IsrDisable(IRQ_LEVEL_MAX);
        result = item->Finished;
        IsrEnable(IRQ_LEVEL_MAX);
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
