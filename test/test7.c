#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/gather.h"
#include"kernel/panic.h"
#include"utils/utils.h"

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

_Bool TransitionArray1[NUM_THREADS];
_Bool TransitionArray2[NUM_THREADS];

//
//Validation
//

_Bool * DoTransition(INDEX index, _Bool * transitionArray)
{
	transitionArray[index] = false;

	if( transitionArray == TransitionArray1 )
	{
		return TransitionArray2;
	}
	else
	{
		return TransitionArray1;
	}
}

void ValidateState(_Bool * transitionArray)
{
	_Bool * checkArray = transitionArray;
	INDEX index;

	//check 
	for(index = 0; index < NUM_THREADS; index++)
	{
		if(checkArray[index])
			KernelPanic();
	}

	for(index=0; index < NUM_THREADS; index++)
		checkArray[index] = true;
}

//
//Blocking main
//

THREAD_MAIN BlockingMain;
void * BlockingMain(void * arg)
{
	INDEX index = (INDEX) arg;
	_Bool * array = TransitionArray1;

	while(true)
	{
		array = DoTransition( index, array );
		GatherSync( & Gather, NULL );
	}
        return NULL;
}

//
//Waiting Main
//

THREAD_MAIN WaitMain;
void * WaitMain(void * arg)
{
	INDEX index = (INDEX) arg;
	_Bool * array = TransitionArray1;
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );

	while(true)
	{
		array = DoTransition( index, array );

		GatherSync( & Gather, &context );

		while( ! LockingIsAcquired( &context ) )
		{
			SchedulerStartCritical();
			SchedulerForceSwitch();
		}
	}
        return NULL;
}

//
//Spinning Main
//

THREAD_MAIN SpinMain;
void * SpinMain(void * arg)
{
	INDEX index = (INDEX) arg;
	_Bool * array = TransitionArray1;
	struct LOCKING_CONTEXT context;

	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking );

	while(true)
	{
		array = DoTransition( index, array );

		GatherSync( & Gather, &context );

		while( !LockingIsAcquired( & context ) );

	}
        return NULL;
}

//
//Validate Main
//

THREAD_MAIN ValidateMain;
void * ValidateMain(void * arg)
{
	INDEX index = (INDEX) arg;
	_Bool * array = TransitionArray1;
	_Bool * check = NULL;

	while(true)
	{
		check = array;
		array = DoTransition( index, array );

		GatherSync( & Gather, NULL );
		
		ValidateState( check );
	}
        return NULL;
}

int main()
{
        INDEX index;

        for(index=0; index < NUM_THREADS; index++)
        {
                TransitionArray1[index] = true;
                TransitionArray2[index] = false;
        }

        KernelInit();

        SchedulerStartup();

        GatherInit( &Gather, NUM_THREADS );

        SchedulerCreateThread(
                        &BlockThread1,
                        1,
                        BlockThread1Stack,
                        STACK_SIZE,
                        BlockingMain,
                        (void *) 0,
                        true);
        SchedulerCreateThread(
                        &BlockThread2,
                        1,
                        BlockThread2Stack,
                        STACK_SIZE,
                        BlockingMain,
                        (void *) 1,
                        true);
        SchedulerCreateThread(
                        &WaitThread1,
                        1,
                        WaitThread1Stack,
                        STACK_SIZE,
                        WaitMain,
                        (void *) 2,
                        true);
        SchedulerCreateThread(
                        &WaitThread2,
                        1,
                        WaitThread2Stack,
                        STACK_SIZE,
                        WaitMain,
                        (void *) 3,
                        true);
        SchedulerCreateThread(
                        &SpinThread1,
                        1,
                        SpinThread1Stack,
                        STACK_SIZE,
                        SpinMain,
                        (void *) 4,
                        true);
        SchedulerCreateThread(
                        &SpinThread2,
                        1,
                        SpinThread2Stack,
                        STACK_SIZE,
                        SpinMain,
                        (void *) 5,
                        true);
        SchedulerCreateThread(
                        &ValidateThread,
                        1,
                        ValidateThreadStack,
                        STACK_SIZE,
                        ValidateMain,
                        (void *) 6,
                        true);

        KernelStart();
        return 0;
}

