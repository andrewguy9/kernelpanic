#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/timer.h"
#include"../kernel/hal.h"
#include"../kernel/sleep.h"
#include"../kernel/panic.h"

//
//Tests whether sleep ever wakes up early.
//

#define ARRAY_SIZE 16
char SleepArray[ARRAY_SIZE] = {3,7,4,1,6,1,8,6,7,2,2,1,3,1,7,1};

void ThreadMain( int index, char bit )
{
	while(1)
	{
		//turn on light
		HalDisableInterrupts();
		DEBUG_LED = DEBUG_LED | bit;
		HalEnableInterrupts();

		//Sleep
		TIME startTime = TimerGetTime();
		Sleep( SleepArray[index] );
		TIME endTime = TimerGetTime();

		//turn off the light
		HalDisableInterrupts();
		DEBUG_LED = DEBUG_LED & (~ bit);
		HalEnableInterrupts();

		//verify time
		if( endTime - startTime < SleepArray[ index ] )
		{
			KernelPanic( PANIC3_THREAD_MAIN_SLEPT_TOO_LITTLE );
		}
		//change the index
		index+=SleepArray[index];
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

int main()
{
	KernelInit();

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
	return 1;
}
