#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/worker.h"
#include"../kernel/interrupt.h"

char WorkerStack[300];
char MainStack[300];

struct THREAD WorkerThread;
struct THREAD MainThread;

void WorkerTask(void *arg)
{
	COUNT * value = (COUNT *) arg;
	(*value)++;
}

COUNT Count;

struct WORKER_ITEM ThreadItem;
void ThreadMain()
{
	COUNT myCount = 0;
	BOOL inUse;

	while(TRUE)
	{
		WorkerAddItem( WorkerTask, &Count, &ThreadItem );
		myCount++;
		do
		{
			InterruptDisable();
			inUse = ThreadItem.Queued;
			InterruptEnable();

			SchedulerStartCritical();
			SchedulerForceSwitch();
		}while( inUse );

		ASSERT( Count == myCount, 
				TEST6_WORK_ITEM_DID_NOT_COMPLETE, 
				"work item didn't complete" );
	}
}

int main()
{
	KernelInit();

	Count = 0;

	SchedulerCreateThread( 
			&MainThread, 
			2, 
			MainStack, 
			300, 
			ThreadMain, 
			0x01 , 
			TRUE );

	WorkerCreateWorker(
			&WorkerThread,
			WorkerStack,
			300,
			0x02 );

	KernelStart();
	return 0;
}
