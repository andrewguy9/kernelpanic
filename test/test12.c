#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/signal.h"
#include"kernel/panic.h"

//#include<stdio.h>

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


#define STACK_SIZE HAL_MIN_STACK_SIZE

char ManagerStack[STACK_SIZE];
char WaiterBlockingStack[STACK_SIZE];
char WaiterNonBlockingStack[STACK_SIZE];

volatile BOOL Flair;
volatile BOOL Respond1;
volatile BOOL Respond2;

//
//Mains
//

THREAD_MAIN ManagerMain;
void ManagerMain(void * unused)
{
	while(1)
	{
		//printf("going to signal\n");
		Flair = TRUE;
		SignalSet( &Signal );
		//printf("signaled\n");
		
		while( !(Respond1 && Respond2) )
		{
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}

		//printf("going to unsignal\n");
		SignalUnset( &Signal );
		//printf("unset\n");

		Flair = FALSE;

		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

THREAD_MAIN WaiterBlockingMain;
void WaiterBlockingMain(void * unused)
{
	while(1)
	{
		Respond1 = FALSE;

		//printf("going to block\n");
		SignalWaitForSignal( & Signal, NULL );
		//printf("woke up\n");

		ASSERT( Flair );
		
		Respond1 = TRUE;

	}
}

THREAD_MAIN WaiterNonBlockingMain;
void WaiterNonBlockingMain(void * unused)
{
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );
	while(1)
	{
		Respond2 = FALSE;
		//printf("going to spin\n");
		SignalWaitForSignal( &Signal, &context );
		while( !LockingIsAcquired( &context ) )
                  ;
		ASSERT( Flair );

		Respond2 = TRUE;
		//printf("spin ended\n");
		
		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

int main()
{
        KernelInit();

        SchedulerStartup();

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
                        TRUE );

        SchedulerCreateThread(
                        &WaiterBlockingThread,
                        1,
                        WaiterBlockingStack,
                        STACK_SIZE,
                        WaiterBlockingMain,
                        NULL,
                        TRUE );

        SchedulerCreateThread(
                        &WaiterNonBlockingThread,
                        1,
                        WaiterNonBlockingStack,
                        STACK_SIZE,
                        WaiterNonBlockingMain,
                        NULL,
                        TRUE );

        KernelStart();
        return 0;
}
