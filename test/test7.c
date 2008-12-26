#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/gather.h"
#include"../kernel/panic.h"
#include"../utils/utils.h"

struct GATHER Gather;

#ifdef PC_BUILD 
#define STACK_SIZE 0x5000
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

struct THREAD BlockThread1;
struct THREAD BlockThread2;

struct THREAD WaitThread1;
struct THREAD WaitThread2;

struct THREAD SpinThread1;
struct THREAD SpinThread2;

char BlockThread1Stack[STACK_SIZE];
char BlockThread2Stack[STACK_SIZE];

char WaitThread1Stack[STACK_SIZE];
char WaitThread2Stack[STACK_SIZE];

char SpinThread1Stack[STACK_SIZE];
char SpinThread2Stack[STACK_SIZE];

//
//Counter
//

COUNT Count;

//
//Blocking main
//

void BlockingMain()
{
	while(TRUE)
	{
		GatherSync( & Gather, NULL );

		SchedulerStartCritical();
		Count++;
		SchedulerEndCritical();
	}
}

//
//Waiting Main
//

void WaitMain()
{
	struct LOCKING_CONTEXT context;
	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );
	while(TRUE)
	{
		GatherSync( & Gather, &context );

		while( ! LockingIsAcquired( &context ) )
		{
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
		SchedulerStartCritical();
		Count++;
		SchedulerEndCritical();
	}
}

//
//Spinning Main
//

void SpinMain()
{
	struct LOCKING_CONTEXT context;
	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );
	while(TRUE)
	{
		GatherSync( & Gather, &context );

		while( !LockingIsAcquired( & context ) );

		SchedulerStartCritical();
		Count++;
		SchedulerEndCritical();
	}
}

int main()
{
	KernelInit();

	Count = 0;

	GatherInit( &Gather, 6 );

	SchedulerCreateThread(
			&BlockThread1,
			1,
		    BlockThread1Stack,
		    STACK_SIZE, 
			BlockingMain,
			NULL,
		    0,
		    TRUE);
	SchedulerCreateThread(
			&BlockThread2,
			1,
		    BlockThread2Stack,
		    STACK_SIZE, 
			BlockingMain,
			NULL,
		    1,
		    TRUE);
	SchedulerCreateThread(
			&WaitThread1,
			1,
		    WaitThread1Stack,
		    STACK_SIZE, 
			WaitMain,
			NULL,
		    2,
		    TRUE);
	SchedulerCreateThread(
			&WaitThread2,
			1,
		    WaitThread2Stack,
		    STACK_SIZE, 
			WaitMain,
			NULL,
		    3,
		    TRUE);
	SchedulerCreateThread(
			&SpinThread1,
			1,
		    SpinThread1Stack,
		    STACK_SIZE, 
			SpinMain,
			NULL,
		    2,
		    TRUE);
	SchedulerCreateThread(
			&SpinThread2,
			1,
		    SpinThread2Stack,
		    STACK_SIZE, 
			SpinMain,
			NULL,
		    3,
		    TRUE);
	KernelStart();
	return 0;
	
}

