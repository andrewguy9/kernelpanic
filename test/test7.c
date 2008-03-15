#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/gather.h"
#include"../kernel/panic.h"
#include"../utils/utils.h"

struct GATHER Gather;

#define STACK_SIZE 300

struct THREAD BlockThread1;
struct THREAD BlockThread2;

struct THREAD WaitThread1;
struct THREAD WaitThread2;

char BlockThread1Stack[STACK_SIZE];
char BlockThread2Stack[STACK_SIZE];
char WaitThread1Stack[STACK_SIZE];
char WaitThread2Stack[STACK_SIZE];

//
//Blocking main
//

void BlockingMain()
{
	while(TRUE)
	{
		GatherSync( & Gather, NULL );
	}
}

//
//Waiting Main
//

void WaitMain()
{
	struct LOCKING_CONTEXT context;
	LockingInit( & context );
	while(TRUE)
	{
		GatherSync( & Gather, &context );

		while( ! LockingIsAcquired( &context ) )
		{
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
	}
}

int main()
{
	KernelInit();

	GatherInit( &Gather, 4 );

	SchedulerCreateThread(
			&BlockThread1,
			1,
		    BlockThread1Stack,
		    STACK_SIZE, 
			BlockingMain,
		    0x01,
		    TRUE);
	SchedulerCreateThread(
			&BlockThread2,
			1,
		    BlockThread2Stack,
		    STACK_SIZE, 
			BlockingMain,
		    0x02,
		    TRUE);
	SchedulerCreateThread(
			&WaitThread1,
			1,
		    WaitThread1Stack,
		    STACK_SIZE, 
			WaitMain,
		    0x04,
		    TRUE);
	SchedulerCreateThread(
			&WaitThread2,
			1,
		    WaitThread2Stack,
		    STACK_SIZE, 
			WaitMain,
		    0x08,
		    TRUE);
	KernelStart();
	return 0;
	
}

