#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/panic.h"

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

#define STACK_SIZE 300

char ProducerStack[STACK_SIZE];
char ConsumerNonBlockingStack[STACK_SIZE];
char ConsumerBlockingStack[STACK_SIZE];

//
//Mains
//

COUNT Produced;
void ProducerMain()
{
	while(1)
	{
		SemaphoreUp( &Lock );
		Produced++;
		SchedulerForceSwitch();
	}
}

COUNT Blocking;
void ConsumerBlockingMain()
{
	while(1)
	{
		SemaphoreDown( &Lock, NULL );
		Blocking++;
	}
}

COUNT NonBlocking;
void ConsumerNonBlockingMain()
{
	struct LOCKING_CONTEXT context;
	LockingInit( & context );
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
	SemaphoreInit( &Lock, 0 );

	SchedulerCreateThread( 
			&Producer, 
			1 , 
			ProducerStack , 
			STACK_SIZE , 
			ProducerMain , 
			0x01 ,
			TRUE );

	SchedulerCreateThread( 
			&ConsumerBlocking, 
			1 , 
			ConsumerBlockingStack , 
			STACK_SIZE , 
			ConsumerBlockingMain,
		   	0x10 , 
			TRUE );

	SchedulerCreateThread(
		   	&ConsumerNonBlocking ,
		   	1 , 
			ConsumerNonBlockingStack , 
			STACK_SIZE , 
			ConsumerNonBlockingMain ,
		   	0x20 , 
			TRUE );

	KernelStart();
	return 0;
}
