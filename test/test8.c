#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/panic.h"
#include"../kernel/interrupt.h"

#include<stdio.h>

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
#define STACK_SIZE (0x5000*20)
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

		/*
		InterruptDisable();
		printf("top\n");
		fflush(stdout);
		InterruptEnable();
		*/

		if( SchedulerIsThreadDead( &DeathThread ) )
		{
			/*
			InterruptDisable();
			printf("create\n");
			fflush(stdout);
			InterruptEnable();
			*/

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

		/*
		InterruptDisable();
		printf("stall\n");
		fflush(stdout);
		InterruptEnable();
		*/

		if( SchedulerIsThreadBlocked( &StallThread ) )
		{
			/*
			InterruptDisable();
			printf("resume\n");
			fflush(stdout);
			InterruptEnable();
			*/
			
			SchedulerResumeThread( &StallThread );

			StallCount--;
		}

		SchedulerEndCritical();

		/*
		InterruptDisable();
		printf("bot\n");
		fflush(stdout);
		InterruptEnable();
		*/
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
	//printf("Death thread %p\n", &DeathThread.MachineContext );
	SchedulerCreateThread(
			&DeathThread,
			1,
			DeathThreadStack,
			STACK_SIZE,
			DeathThreadMain,
			NULL,
		   	4,
			TRUE);

	//printf("Stall Thread %p\n", &StallThread.MachineContext );
	SchedulerCreateThread(
			&StallThread,
			1,
			StallThreadStack,
			STACK_SIZE,
			StallThreadMain,
			NULL,
			5,
			TRUE);

	//printf("Restart Thread %p\n", &RestartThread.MachineContext );
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
