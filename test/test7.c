#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/gather.h"
#include"../kernel/panic.h"
#include"../utils/utils.h"

#include<stdio.h>

#define NUM_THREADS 7

struct GATHER Gather;

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct THREAD BlockThread1;
struct THREAD BlockThread2;

struct THREAD WaitThread1;
struct THREAD WaitThread2;

struct THREAD SpinThread1;
struct THREAD SpinThread2;

struct THREAD ValidateThread;

char BlockThread1Stack[STACK_SIZE];
char BlockThread2Stack[STACK_SIZE];

char WaitThread1Stack[STACK_SIZE];
char WaitThread2Stack[STACK_SIZE];

char SpinThread1Stack[STACK_SIZE];
char SpinThread2Stack[STACK_SIZE];

char ValidateThreadStack[STACK_SIZE];

BOOL TransitionArray1[NUM_THREADS];
BOOL TransitionArray2[NUM_THREADS];

//
//Validation
//

BOOL * DoTransition(INDEX index, BOOL * transitionArray)
{
	transitionArray[index] = FALSE;

	if( transitionArray == TransitionArray1 )
	{
		return TransitionArray2;
	}
	else
	{
		return TransitionArray1;
	}
}

void ValidateState(BOOL * transitionArray)
{
	BOOL * checkArray = transitionArray;
	INDEX index;

	//check 
	for(index = 0; index < NUM_THREADS; index++)
	{
		if(checkArray[index])
			KernelPanic();
	}

	for(index=0; index < NUM_THREADS; index++)
		checkArray[index] = TRUE;
}

//
//Blocking main
//

void BlockingMain(void * arg)
{
	INDEX index = (INDEX) arg;
	BOOL * array = TransitionArray1;

	while(TRUE)
	{
		array = DoTransition( index, array );
		GatherSync( & Gather, NULL );
	}
}

//
//Waiting Main
//

void WaitMain(void * arg)
{
	INDEX index = (INDEX) arg;
	BOOL * array = TransitionArray1;
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );

	while(TRUE)
	{
		array = DoTransition( index, array );

		GatherSync( & Gather, &context );

		while( ! LockingIsAcquired( &context ) )
		{
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
	}
}

//
//Spinning Main
//

void SpinMain(void * arg)
{
	INDEX index = (INDEX) arg;
	BOOL * array = TransitionArray1;
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );

	while(TRUE)
	{
		array = DoTransition( index, array );

		GatherSync( & Gather, &context );

		while( !LockingIsAcquired( & context ) );

	}
}

//
//Validate Main
//

void ValidateMain(void * arg)
{
	INDEX index = (INDEX) arg;
	BOOL * array = TransitionArray1;
	BOOL * check = NULL;

	while(TRUE)
	{
		check = array;
		array = DoTransition( index, array );

		GatherSync( & Gather, NULL );
		
		ValidateState( check );
	}
}

int main()
{
	INDEX index;

	for(index=0; index < NUM_THREADS; index++)
	{
		TransitionArray1[index] = TRUE;
		TransitionArray2[index] = FALSE;
	}

	KernelInit();

	GatherInit( &Gather, NUM_THREADS );

	SchedulerCreateThread(
			&BlockThread1,
			1,
		    BlockThread1Stack,
		    STACK_SIZE, 
			BlockingMain,
			(void *) 0,
		    0,
		    TRUE);
	SchedulerCreateThread(
			&BlockThread2,
			1,
		    BlockThread2Stack,
		    STACK_SIZE, 
			BlockingMain,
			(void *) 1,
		    1,
		    TRUE);
	SchedulerCreateThread(
			&WaitThread1,
			1,
		    WaitThread1Stack,
		    STACK_SIZE, 
			WaitMain,
			(void *) 2,
		    2,
		    TRUE);
	SchedulerCreateThread(
			&WaitThread2,
			1,
		    WaitThread2Stack,
		    STACK_SIZE, 
			WaitMain,
			(void *) 3,
		    3,
		    TRUE);
	SchedulerCreateThread(
			&SpinThread1,
			1,
		    SpinThread1Stack,
		    STACK_SIZE, 
			SpinMain,
			(void *) 4,
		    2,
		    TRUE);
	SchedulerCreateThread(
			&SpinThread2,
			1,
		    SpinThread2Stack,
		    STACK_SIZE, 
			SpinMain,
			(void *) 5,
		    3,
		    TRUE);
	SchedulerCreateThread(
			&ValidateThread,
			1, 
			ValidateThreadStack,
			STACK_SIZE,
			ValidateMain,
			(void *) 6,
			4,
			TRUE);

	KernelStart();
	return 0;
	
}

