#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/timer.h"
#include"../kernel/hal.h"
#include"../kernel/sleep.h"
#include"../kernel/panic.h"
//
//Tests whether semaphore ever admits more entries than it should.
//

#define SEMAPHORE_SIZE 10
struct SEMAPHORE Semaphore;

#define ARRAY_SIZE 16
char WorkArray[ARRAY_SIZE] = {3,7,4,1,6,1,8,6,7,2,2,1,3,1,7,1};
char HoldArray[ARRAY_SIZE] =  {5,4,3,6,2,9,1,6,1,4,2,5,8,7,4,9};

void Work( COUNT work)
{
	COUNT done = 0;
	TIME time = TimerGetTime();
	while( done <= work  )
	{
		//loop until time changes
		while( time == TimerGetTime() );

		time = TimerGetTime();
		done++;
	}
}

void ThreadMain( int index, char bit, int * semaphoreUsage )
{
	while(1)
	{
		//turn on light
		HalDisableInterrupts();
		DEBUG_LED = DEBUG_LED | bit;
		HalEnableInterrupts();

		//aquire lock
		SemaphoreLock( & Semaphore, HoldArray[index] );
		HalDisableInterrupts();
		* semaphoreUsage = HoldArray[index];
		HalEnableInterrupts();

		//do some "work"
		Work( WorkArray[index] );

		//relase the lock
		HalDisableInterrupts();
		* semaphoreUsage = HoldArray[index];
		HalEnableInterrupts();
		SemaphoreUnlock( & Semaphore, HoldArray[index] );

		//turn off the light
		HalDisableInterrupts();
		DEBUG_LED = DEBUG_LED & (~ bit);
		HalEnableInterrupts();

		//change the index
		index+=WorkArray[index]+HoldArray[index];
		index%=ARRAY_SIZE;
	}
}

struct THREAD Thread1;
char Thread1Stack[200];
int Thread1SemaphoreCount;
void Thread1Main()
{
	Thread1SemaphoreCount = 0;
	ThreadMain( 1, 0x01, &Thread1SemaphoreCount );
}

struct THREAD Thread2;
char Thread2Stack[200];
int Thread2SemaphoreCount;
void Thread2Main()
{
	Thread2SemaphoreCount = 0;
	ThreadMain( 2, 0x02, &Thread2SemaphoreCount );
}

struct THREAD Thread3;
char Thread3Stack[200];
int Thread3SemaphoreCount;
void Thread3Main()
{
	Thread3SemaphoreCount = 0;
	ThreadMain( 3, 0x04, &Thread3SemaphoreCount );
}

struct TIMER VerificationTimer;
void VerifyState()
{

	//Check to make sure semaphore is utilized properly
	int total = Thread1SemaphoreCount+
		Thread2SemaphoreCount+
		Thread3SemaphoreCount;
	if( total > SEMAPHORE_SIZE )
	{
		KernelPanic( PANIC2_VALIDATE_STATE_SEMAPHORE_OVERUSED );
	}

	TimerRegister(
		&VerificationTimer,
		1,
		VerifyState,
		NULL);
}

int main()
{
	KernelInit();

	//initialize verificatio timer
	TimerRegister(
			&VerificationTimer,
			1,
			VerifyState,
			NULL);

	//intialize semaphore
	SemaphoreInit( & Semaphore, SEMAPHORE_SIZE );

	//create threads
	SchedulerCreateThread(
			&Thread1,
			5,
			Thread1Stack,
			200,
			Thread1Main);
	SchedulerCreateThread(
			&Thread2,
			10,
			Thread2Stack,
			200,
			Thread2Main);
	SchedulerCreateThread(
			&Thread3,
			15,
			Thread3Stack,
			200,
			Thread3Main);

	KernelStart();
}
