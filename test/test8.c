#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/panic.h"
#include"../kernel/interrupt.h"

/*
 * Demonstrates how to start and stop a thread.
 */

//
//Thread structures
//

#ifdef PC_BUILD 
#define STACK_SIZE (0x5000*2)
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

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

void DeathThreadMain( void * arg )
{
	if( DeathCount != 0 )
	{
		KernelPanic();
	}

	DeathCount++;
	TotalDeath++;
}

void StallThreadMain( void * arg )
{
	while(TRUE)
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
}

void RestartThreadMain( void * arg )
{
	while(TRUE)
	{
		SchedulerStartCritical();

		if( SchedulerIsThreadDead( &DeathThread ) )
		{
			SchedulerCreateThread(
					&DeathThread,
					1,
					DeathThreadStack,
					STACK_SIZE,
					DeathThreadMain,
					NULL,
					4,
					TRUE);

			DeathCount--;
		}

		if( SchedulerIsThreadBlocked( &StallThread ) )
		{
			SchedulerResumeThread( &StallThread );

			StallCount--;
		}

		SchedulerEndCritical();
	}
}

//
//Main
//

int main()
{
	//Initialize the kernel structures.
	KernelInit();

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
		   	4,
			TRUE);

	SchedulerCreateThread(
			&StallThread,
			1,
			StallThreadStack,
			STACK_SIZE,
			StallThreadMain,
			NULL,
			5,
			TRUE);

	SchedulerCreateThread(
			&RestartThread,
			1,
			RestartThreadStack,
			STACK_SIZE,
			RestartThreadMain,
			NULL,
			6,
			TRUE);

	KernelStart();
	return 0;
}
