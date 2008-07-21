#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/resource.h"
#include"../kernel/hal.h"
#include"../kernel/sleep.h"
#include"../kernel/panic.h"

/*
 * Test of the resource unit and the non blocking code path.
 * Should panic on failure.
 */

//
//Tests Resources
//

#define BUFFER_SIZE 512
int Buffer[BUFFER_SIZE];
#define SEQUENCE_LENGTH 16
int Sequence[SEQUENCE_LENGTH]={1,3,5,7,11,13,17,19,23,27,29,31,37,39,41,43};

struct RESOURCE BufferLock;

COUNT TimesWritten;
COUNT TimesRead;

void Writer( void * arg )
{
	struct LOCKING_CONTEXT block;
	INDEX sequenceIndex=0;
	INDEX index;

	LockingInit( &block, LockingBlockNonBlocking, LockingWakeNonBlocking );

	while(1)
	{
		ResourceLockExclusive( &BufferLock, &block);

		while( !LockingIsAcquired( &block ) );
	
		//resource should be exlusive
		ASSERT( BufferLock.State == RESOURCE_EXCLUSIVE );

		sequenceIndex++;
		sequenceIndex%=SEQUENCE_LENGTH;

		for( index = 0; index < BUFFER_SIZE; index++ )
		{
			Buffer[index] = index + Sequence[ sequenceIndex ] ;
		}
		
		ResourceUnlockExclusive( &BufferLock );
		
		SchedulerStartCritical();
		TimesWritten++;
		SchedulerForceSwitch();
	}
}

void Reader( void * arg )
{
	INDEX index;

	int first,second;
	BOOL ready = FALSE;

	while( ! ready )
	{
		SchedulerStartCritical();
		if( TimesWritten > 0 )
		{
			ready = TRUE;
			SchedulerEndCritical();
		}
		else
		{
			SchedulerForceSwitch();	
		}
	}

	while(1)
	{
		ResourceLockShared( &BufferLock, NULL );

		//the resource should be shared
		ASSERT( BufferLock.State == RESOURCE_SHARED );

		for(index=1 ; index < BUFFER_SIZE; index++)
		{
			first = Buffer[index-1];
			second = Buffer[index];
			if( (first +1) != second )
			{
				KernelPanic( );
			}
		}

		ResourceUnlockShared( &BufferLock );

		SchedulerStartCritical();
		TimesRead++;
		SchedulerForceSwitch();
	}
}

struct THREAD Writer1;
struct THREAD Writer2;
struct THREAD Reader1;
struct THREAD Reader2;
struct THREAD Reader3;

#define STACK_SIZE 400
char Writer1Stack[STACK_SIZE];
char Writer2Stack[STACK_SIZE];
char Reader1Stack[STACK_SIZE];
char Reader2Stack[STACK_SIZE];
char Reader3Stack[STACK_SIZE];

int main()
{
	KernelInit();

	TimesRead = 0;
	TimesWritten = 0;

	SchedulerCreateThread( 
			& Reader1,
			5,
			Reader1Stack,
			STACK_SIZE,
			Reader,
			NULL,
			0,
			TRUE);
	SchedulerCreateThread( 
			& Reader2,
			5,
			Reader2Stack,
			STACK_SIZE,
			Reader,
			NULL,
			1,
			TRUE);
	SchedulerCreateThread( 
			& Reader3,
			5,
			Reader3Stack,
			STACK_SIZE,
			Reader,
			NULL,
			2,
			TRUE);
	SchedulerCreateThread( 
			& Writer1,
			5,
			Writer1Stack,
			STACK_SIZE,
			Writer,
			NULL,
			4,
			TRUE);
	SchedulerCreateThread( 
			& Writer2,
			5,
			Writer2Stack,
			STACK_SIZE,
			Writer,
			NULL,
			5,
			TRUE);

	KernelStart();
	return 0;
}
