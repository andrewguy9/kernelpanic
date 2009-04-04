#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/signal.h"
#include"../kernel/panic.h"

#include<stdio.h>

/*
 * Tests the signal unit. 
 *
 * Will ASSERT on failure.
 */

//
//Signal
//

struct SIGNAL Signal;

//
//Threads
//

struct THREAD ManagerThread;
struct THREAD WaiterBlockingThread;
struct THREAD WaiterNonBlockingThread;

//
//Stacks
//


#ifdef PC_BUILD 
#define STACK_SIZE 0x5000
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

char ManagerStack[STACK_SIZE];
char WaiterBlockingStack[STACK_SIZE];
char WaiterNonBlockingStack[STACK_SIZE];

volatile BOOL Flair;
volatile BOOL Respond1;
volatile BOOL Respond2;

//
//Mains
//

void ManagerMain()
{
	while(1)
	{
		printf("going to signal\n");
		Flair = TRUE;
		SignalSet( &Signal );
		printf("signaled\n");
		
		while( !(Respond1 && Respond2) )
		{
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}

		printf("going to unsignal\n");
		SignalUnset( &Signal );
		printf("unset\n");

		Flair = FALSE;

		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

void WaiterBlockingMain()
{
	while(1)
	{
		Respond1 = FALSE;

		printf("going to block\n");
		SignalWaitForSignal( & Signal, NULL );
		printf("woke up\n");

		ASSERT( Flair );
		
		Respond1 = TRUE;

	}
}

void WaiterNonBlockingMain()
{
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );
	while(1)
	{
		Respond2 = FALSE;
		printf("going to spin\n");
		SignalWaitForSignal( &Signal, &context );
		while( !LockingIsAcquired( &context ) );
		ASSERT( Flair );

		Respond2 = TRUE;
		printf("spin ended\n");
		
		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

int main()
{
	KernelInit();

	SignalInit( &Signal, FALSE );

	Flair = FALSE;
	Respond1 = FALSE;
	Respond2 = FALSE;

	SchedulerCreateThread(
			&ManagerThread,
			1,
			ManagerStack,
			STACK_SIZE,
			ManagerMain,
			NULL,
			0,
			TRUE );

	SchedulerCreateThread(
			&WaiterBlockingThread,
			1,
			WaiterBlockingStack,
			STACK_SIZE,
			WaiterBlockingMain,
			NULL,
			0,
			TRUE );

	SchedulerCreateThread(
			&WaiterNonBlockingThread,
			1,
			WaiterNonBlockingStack,
			STACK_SIZE,
			WaiterNonBlockingMain,
			NULL,
			0,
			TRUE );

	KernelStart();
	return 0;
}
