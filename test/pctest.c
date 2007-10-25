#include"../kernel/scheduler.h"
#include"../utils/linkedlist.h"
#include<stdio.h>

struct THREAD Inc;
struct THREAD Div;
struct THREAD IdleLoop;

struct LINKED_LIST Queue1;
struct LINKED_LIST Queue2;

struct LINKED_LIST * RunQueue;
struct LINKED_LIST * DoneQueue;

struct THREAD * ActiveThread = & IdleLoop;
struct THREAD * NextThread = NULL;

void Run( struct THREAD * thread )
{
	printf("Running ");
	if( thread == & Inc )
		printf("increment");
	else if( thread == & Div )
		printf("divide");
	else if( thread == & IdleLoop )
		printf("idle loop");
	printf("\n");
}

void Schedule( )
{
	printf("Scheduling\n");
	if( ActiveThread != &IdleLoop && 
			ActiveThread->State == THREAD_STATE_RUNNING)
	{
		printf("\tAdding to done queue\n");
		LinkedListEnqueue( (struct LINKED_LIST_LINK *) ActiveThread,
				DoneQueue);
	}

	if( LinkedListIsEmpty( RunQueue ) )
	{
		//There are no threads in run queue.
		//This is a sign we have run through
		//all threads, so well pull from done
		//queue now
		printf("\trun queue empty, switching\n");
		struct LINKED_LIST * temp = RunQueue;
		RunQueue = DoneQueue;
		DoneQueue = temp;
	}

	//Pick the next thread
	if( ! LinkedListIsEmpty( RunQueue ) )
	{//there are threads waiting, run one
		printf("\tselected another thread\n");
		NextThread = 
			(struct THREAD * ) LinkedListPop( RunQueue );
	}
	else
	{//there were no threads at all, use idle loop.
		printf("\tno threads, using idle\n");
		NextThread = &IdleLoop;
	}
}

int main()
{
	char input;
	//Init
	LinkedListInit( RunQueue );
	LinkedListInit( DoneQueue );

	RunQueue = & Queue1;
	DoneQueue = & Queue2;

	LinkedListEnqueue( (struct LINKED_LIST_LINK *) &Inc, DoneQueue );
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) &Div, DoneQueue );
	//loop
	while( 1 )
	{
		Run( ActiveThread );
		Schedule();
		if( NextThread != NULL )
		{
			printf("switch\n");
			ActiveThread = NextThread;
			NextThread = NULL;
		}
		printf("\n\n");
	}
}
