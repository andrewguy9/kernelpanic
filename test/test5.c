#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/panic.h"

/*
 * Tests the semaphore unit, both blocking and non blocking unit.
 * Over time Produced = (Blocking+NonBlocking)+-1.
 * Will panic on failure.
 */

//
//Semaphores
//

struct SEMAPHORE Lock;

//
//Threads
//

struct THREAD Producer;
struct THREAD ConsumerBlocking;
struct THREAD ConsumerNonBlocking;

//
//Stacks
//

#define STACK_SIZE HAL_MIN_STACK_SIZE

char ProducerStack[STACK_SIZE];
char ConsumerNonBlockingStack[STACK_SIZE];
char ConsumerBlockingStack[STACK_SIZE];

//
//State
//

COUNT Produced;
COUNT Blocking;
COUNT NonBlocking;

//
//Mains
//

void ProducerMain()
{
	while(1)
	{
		SemaphoreUp( &Lock );
		Produced++;
		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
}

void ConsumerBlockingMain()
{
	while(1)
	{
		SemaphoreDown( &Lock, NULL );
		Blocking++;
	}
}

void ConsumerNonBlockingMain()
{
	struct LOCKING_CONTEXT context;
	LockingInit( & context, LockingBlockNonBlocking, LockingWakeNonBlocking ); 
	while(1)
	{
		SemaphoreDown( &Lock, &context );
		while( !LockingIsAcquired( &context ) );
		NonBlocking++;
	}
}

int main()
{
	KernelInit();
	
	SchedulerStartup();

	SemaphoreInit( &Lock, 0 );

	SchedulerCreateThread( 
			&Producer, 
			1 , 
			ProducerStack , 
			STACK_SIZE , 
			ProducerMain , 
			NULL,
			0,
			TRUE );

	SchedulerCreateThread( 
			&ConsumerBlocking, 
			1 , 
			ConsumerBlockingStack , 
			STACK_SIZE , 
			ConsumerBlockingMain,
			NULL,
		   	4, 
			TRUE );

	SchedulerCreateThread(
		   	&ConsumerNonBlocking ,
		   	1 , 
			ConsumerNonBlockingStack , 
			STACK_SIZE , 
			ConsumerNonBlockingMain ,
			NULL,
		   	5, 
			TRUE );

	KernelStart();
	return 0;
}
