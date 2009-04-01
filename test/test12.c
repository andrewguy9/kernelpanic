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

//
//Mains
//

COUNT TimesSet = 0;
void ManagerMain()
{
	while(1)
	{
		printf("going to signal\n");
		SignalSet( &Signal );
		printf("signaled\n");
		TimesSet++;
		//TODO WAIT
		
		SchedulerStartCritical();
		SchedulerForceSwitch();

		printf("going to unsignal\n");
		SignalUnset( &Signal );
		printf("unset\n");
		//TODO WAIT

		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

COUNT TimesBlockingSignaled = 0;
void WaiterBlockingMain()
{
	while(1)
	{
		printf("going to block\n");
		SignalWaitForSignal( & Signal, NULL );
		printf("woke up\n");
		TimesBlockingSignaled++;
		//TODO COUNT
		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

COUNT TimesNonBlockingSignaled = 0;
void WaiterNonBlockingMain()
{
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );
	while(1)
	{
		printf("going to spin\n");
		SignalWaitForSignal( &Signal, &context );
		while( !LockingIsAcquired( &context ) );
		TimesNonBlockingSignaled++;
		printf("spin ended\n");
		//TODO COUNT
		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

int main()
{
	KernelInit();

	SignalInit( &Signal, FALSE );

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
