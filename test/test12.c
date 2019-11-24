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

volatile _Bool Flair;
volatile _Bool Respond1;
volatile _Bool Respond2;

//
//Mains
//

THREAD_MAIN ManagerMain;
void * ManagerMain(void * unused)
{
	while(1)
	{
		//printf("going to signal\n");
		Flair = true;
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

		Flair = false;

		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
        return NULL;
}

THREAD_MAIN WaiterBlockingMain;
void * WaiterBlockingMain(void * unused)
{
	while(1)
	{
		Respond1 = false;

		//printf("going to block\n");
		SignalWaitForSignal( & Signal, NULL );
		//printf("woke up\n");

		ASSERT( Flair );
		
		Respond1 = true;

	}
        return NULL;
}

THREAD_MAIN WaiterNonBlockingMain;
void * WaiterNonBlockingMain(void * unused)
{
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );
	while(1)
	{
		Respond2 = false;
		//printf("going to spin\n");
		SignalWaitForSignal( &Signal, &context );
		while( !LockingIsAcquired( &context ) )
                  ;
		ASSERT( Flair );

		Respond2 = true;
		//printf("spin ended\n");
		
		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
        return NULL;
}

int main()
{
        KernelInit();

        SchedulerStartup();

        SignalInit( &Signal, false );

        Flair = false;
        Respond1 = false;
        Respond2 = false;

        SchedulerCreateThread(
                        &ManagerThread,
                        1,
                        ManagerStack,
                        STACK_SIZE,
                        ManagerMain,
                        NULL,
                        true );

        SchedulerCreateThread(
                        &WaiterBlockingThread,
                        1,
                        WaiterBlockingStack,
                        STACK_SIZE,
                        WaiterBlockingMain,
                        NULL,
                        true );

        SchedulerCreateThread(
                        &WaiterNonBlockingThread,
                        1,
                        WaiterNonBlockingStack,
                        STACK_SIZE,
                        WaiterNonBlockingMain,
                        NULL,
                        true );

        KernelStart();
        return 0;
}
