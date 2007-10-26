#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/timer.h"
#include"../kernel/hal.h"
#include"../kernel/sleep.h"

struct SEMAPHORE Semaphore;

#define ARRAY_SIZE 16
char SleepArray[ARRAY_SIZE] = {3,7,4,1,6,1,8,6,7,2,2,1,3,1,7,1};
char HoldArray[ARRAY_SIZE] =  {5,4,3,6,2,9,1,6,1,4,2,5,8,7,4,9};


void ThreadMain( int index, char bit )
{
	while(1)
	{
		//turn on light
		HalDisableInterrupts();
		DEBUG_LED = DEBUG_LED | bit;
		HalEnableInterrupts();

		//aquire lock
		SemaphoreLock( & Semaphore, HoldArray[index] );

		//do some "work"
		Sleep( SleepArray[index] );

		//relase the lock
		SemaphoreUnlock( & Semaphore, HoldArray[index] );

		//turn off the light
		HalDisableInterrupts();
		DEBUG_LED = DEBUG_LED & (~ bit);
		HalEnableInterrupts();

		//change the index
		index+=SleepArray[index]+HoldArray[index];
		index%=ARRAY_SIZE;
	}
}

struct THREAD Thread1;
char Thread1Stack[200];
void Thread1Main()
{
	ThreadMain( 1, 0x01 );
}

struct THREAD Thread2;
char Thread2Stack[200];
void Thread2Main()
{
	ThreadMain( 2, 0x02 );
}

struct THREAD Thread3;
char Thread3Stack[200];
void Thread3Main()
{
	ThreadMain( 3, 0x04 );
}

struct TIMER VerificationTimer;
void VerifyState()
{
	//TODO
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
	SemaphoreInit( & Semaphore, 10 );

	//create threads
	SchedulerCreateThread(
			&Thread1,
			10,
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
			10,
			Thread3Stack,
			200,
			Thread3Main);

	KernelStart();
}
