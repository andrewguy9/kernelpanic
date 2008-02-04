#include"worker.h"
#include"../utils/linkedlist.h"
#include"interrupt.h"

struct LINKED_LIST WorkerItemQueue;

WorkerStartup()
{
	LinkedListInit( &TaskList );	
}

void WorkerThreadMain()
{
	struct HANDLER_OBJECT * item;
	HANDLER handler;
	void * arg;

	while(TRUE)
	{
		//Fetch a item
		InterruptDisable();
		item = LinkedListPop( &WorkerItemList );
		InterruptEnable();

		if( item == NULL )
		{//there is no item, lets switch threads
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
		else
		{//there is a item, so execute.
			handler = item->Handler;
			arg = item->Argument;
			item->Enabled = FALSE;

			handler( arg );
		}
	}
}

void CreateWorkerThread(
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
			true);
}

void WorkerAddItem( HANDLER foo, void * arg, struct HANDLER_OBJECT * obj  )
{
	InterruptDisable();

	obj->Enabled = TRUE;
	obj->Handler = foo;
	obj->Argument = arg;

	LinkedListEnqueue( &obj->Link.LinkedListLink, &WorkerItemQueue );
	
	InterruptEnable();
}
