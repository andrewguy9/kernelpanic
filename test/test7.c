#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/gather.h"
#include"../kernel/panic.h"
#include"../utils/utils.h"

struct GATHER Gather;

#define STACK_SIZE 300

struct THREAD BlockThread1;
struct THREAD BlockThread2;

struct THREAD WaitThread3;
struct THREAD WaitThread4;

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
