#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/panic.h"

/*
 * Demonstrates how to start and stop a thread.
 */

//
//Thread structures
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD DeathThread;
char DeathThreadStack[STACK_SIZE];

struct THREAD StallThread;
char StallThreadStack[STACK_SIZE];

struct THREAD RestartThread;
char RestartThreadStack[STACK_SIZE];

//
//Validation
//

volatile int DeathCount;
volatile int TotalDeath;
volatile int StallCount;
volatile int TotalStall;

//
//Main routine for threads.
//

THREAD_MAIN DeathThreadMain;
void * DeathThreadMain( void * arg )
{
	if( DeathCount != 0 )
	{
		KernelPanic();
	}

	DeathCount++;
	TotalDeath++;
        return NULL;
}

THREAD_MAIN StallThreadMain;
void * StallThreadMain( void * arg )
{
	while(true)
	{
		if(StallCount != 0 )
		{
			KernelPanic();
		}

		StallCount++;
		TotalStall++;
		SchedulerStartCritical();
		SchedulerBlockThread();
		SchedulerForceSwitch();
	}

	//We should never get here.
	KernelPanic();
        return NULL;
}

THREAD_MAIN RestartThreadMain;
void * RestartThreadMain( void * arg )
{
        while(true)
        {
                SchedulerJoinThread(&DeathThread);
                SchedulerStartCritical();
                ASSERT( SchedulerIsThreadDead( &DeathThread ) );
                SchedulerCreateThread(
                    &DeathThread,
                    1,
                    DeathThreadStack,
                    STACK_SIZE,
                    DeathThreadMain,
                    NULL,
                    true);
                DeathCount--;

                if( SchedulerIsThreadBlocked( &StallThread ) ) {
                        SchedulerResumeThread( &StallThread );
                        StallCount--;
                }
                SchedulerEndCritical();
        }
        return NULL;
}

//
//Main
//

int main()
{
        //Initialize the kernel structures.
        KernelInit();

        SchedulerStartup();

        //Initialize variables
        DeathCount = 0;
        TotalDeath = 0;
        StallCount = 0;
        TotalStall = 0;

        //Initialize Threads
        SchedulerCreateThread(
                        &DeathThread,
                        1,
                        DeathThreadStack,
                        STACK_SIZE,
                        DeathThreadMain,
                        NULL,
                        true);

        SchedulerCreateThread(
                        &StallThread,
                        1,
                        StallThreadStack,
                        STACK_SIZE,
                        StallThreadMain,
                        NULL,
                        true);

        SchedulerCreateThread(
                        &RestartThread,
                        1,
                        RestartThreadStack,
                        STACK_SIZE,
                        RestartThreadMain,
                        NULL,
                        true);

        KernelStart();
        return 0;
}
