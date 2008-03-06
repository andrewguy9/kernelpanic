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
	struct HANDLER_OBJECT * handler;
	struct WORKER_ITEM * item;

	while(TRUE)
	{
		//Fetch a handler
		InterruptDisable();
		handler = BASE_OBJECT( 
				LinkedListPop( &WorkerItemQueue ),
				struct HANDLER_OBJECT,
				Link.LinkedListLink );
		InterruptEnable();

		//cast into worker item
		item = BASE_OBJECT(
				handler,
				struct WORKER_ITEM,
				Handler);

		if( handler == NULL )
		{//there is no item, lets switch threads
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
		else
		{//there is a item, so execute it.
			ASSERT( item->Queued,
					WORKER_HANDLER_NOT_QUEUED,
					"We need the handler to be ready for running");

			//mark handler for use
			item->Queued = FALSE;
			//run handler
			item->Handler.Function(item);
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

void WorkerAddItem( HANDLER_FUNCTION foo, void * context, struct WORKER_ITEM * item  )
{
	InterruptDisable();

	item->Queued = TRUE;
	item->Handler.Function = foo;
	item->Context = context;

	LinkedListEnqueue( &item->Handler.Link.LinkedListLink, &WorkerItemQueue );
	
	InterruptEnable();
}
