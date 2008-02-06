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
	*value ++;
}

void ThreadMain()
{
	struct HANDLER_OBJECT handler;
	COUNT count = 0;
	COUNT myCount = 0;
	BOOL inUse;

	while(TRUE)
	{
		WorkerAddItem( WorkerTask, NULL, &handler );
		myCount++;
		do
		{
			InterruptDisable();
			inUse = handler.Enabled;
			InterruptEnable();

			SchedulerStartCritical();
			SchedulerForceSwitch();
		}while( ! inUse );

		ASSERT( count == myCount, 0, "work item didn't complete" );
	}
}

int main()
{
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
