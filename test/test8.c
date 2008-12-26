#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/hal.h"
#include"../kernel/panic.h"

/*
 * Demonstrates how to start and stop a thread.
 */

//
//Vars 
//

int DeathCount;
int TotalDeath;
int StallCount;
int TotalStall;

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
//Main routine for threads.
//

void DeathThreadMain( void * arg )
{
	ASSERT(DeathCount == 0 );

	DeathCount++;
	TotalDeath++;
}

void StallThreadMain( void * arg )
{
	while(TRUE)
	{
		ASSERT(StallCount == 0 );

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
