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
	struct HANDLER_OBJECT * item;
	HANDLER_FUNCTION * handler;
	void * arg;

	while(TRUE)
	{
		//Fetch a item
		InterruptDisable();
		item = LinkedListPop( &WorkerItemQueue );
		InterruptEnable();

		if( item == NULL )
		{//there is no item, lets switch threads
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
		else
		{//there is a item, so execute it.
			handler = item->Handler;
			arg = item->Argument;
			item->Enabled = FALSE;

			handler( arg );
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

void WorkerAddItem( HANDLER_FUNCTION foo, void * arg, struct HANDLER_OBJECT * obj  )
{
	InterruptDisable();

	obj->Enabled = TRUE;
	obj->Handler = foo;
	obj->Argument = arg;

	LinkedListEnqueue( &obj->Link.LinkedListLink, &WorkerItemQueue );
	
	InterruptEnable();
}
