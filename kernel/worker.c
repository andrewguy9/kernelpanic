#include"worker.h"
#include"../utils/linkedlist.h"
#include"interrupt.h"
#include"scheduler.h"

struct LINKED_LIST WorkerItemQueue;

void WorkerStartup()
{
	LinkedListInit( &WorkerItemQueue );	
}

void WorkerThreadMain()
{
	struct WORKER_ITEM * item;
	enum WORKER_RETURN result;
	
	while(TRUE)
	{
		//Fetch a handler
		InterruptDisable();
		item = BASE_OBJECT( LinkedListPop( &WorkerItemQueue ),
				struct WORKER_ITEM,
				Link);
		InterruptEnable();

		if( item == NULL )
		{//there is no item, lets switch threads
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
		else
		{//there is a item, so execute it.
			ASSERT( ! item->Finished,
					WORKER_HANDLER_FINISHED,
					"The handler finished, and got scheduled anyway.");

			//run handler
			result = item->Foo(item);

			//
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
			flag,
			TRUE);
}

void WorkerAddItem( WORKER_FUNCTION foo, void * context, struct WORKER_ITEM * item  )
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
