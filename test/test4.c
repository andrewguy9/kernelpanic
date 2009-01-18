#include"../kernel/sleep.h"
#include"../kernel/scheduler.h"
#include"../kernel/timer.h"
#include"../utils/utils.h"
#include"../kernel/startup.h"
#include"../kernel/interrupt.h"
#include"../kernel/panic.h"
#include"../kernel/worker.h"

/*
 * Tests the sleep unit, should panic on failure.
 */

//Define Sleep Patern
#define SEQUENCE_LENGTH 8
COUNT Sequence[SEQUENCE_LENGTH] = {2,4,8,16,32,64,128,256};

//Define Thread
struct THREAD SleeperThread;
struct THREAD WorkerThread;

#ifdef PC_BUILD 
#define STACK_SIZE 0x5000
#endif

#ifdef AVR_BUILD
#define STACK_SIZE 0x500
#endif

char SleeperStack[STACK_SIZE];
char WorkerStack[STACK_SIZE];

//Define Timer
struct POST_HANDLER_OBJECT Timer;

//Define Global Flags
volatile BOOL TimerFlag;//Is TRUE when we have the timer registered. (approx)
volatile BOOL ThreadFlag;//Is TRUE when sleeping, FALSE when awake (approx)
COUNT TimerCycles;//Times we have run the test.

//TimerFunction
void TimerHandler( void * Argument )
{
	//Clear Flag
	TimerFlag = FALSE;
	//Check to see if thread is sleeping
	if( ! ThreadFlag )
		KernelPanic( );
}

//Thread Main
void SleeperMain()
{
	INDEX cur=0;
	while(1)
	{
		for( cur = 0; cur < SEQUENCE_LENGTH; cur++)
		{
			//Register Timer: The timer should run before we wake.
			InterruptDisable();
			TimerFlag = TRUE;
			InterruptEnable();
			TimerRegister(
					& Timer,
					Sequence[cur] - 1,
					TimerHandler,
					NULL);

			//Go to sleep:
			InterruptDisable();
			ThreadFlag = TRUE;
			InterruptEnable();
			Sleep( Sequence[cur] );

			//Now that we are awake, Clear the thread flag.
			InterruptDisable();
			ThreadFlag = FALSE;
			InterruptEnable();

			//Check to see if the timer fired before we woke.
			InterruptDisable();
			if( TimerFlag )
			{
				KernelPanic( );
			}
			InterruptEnable();

			//Increase our iteration count.
			TimerCycles++;
		}
	}
}

int main()
{
	KernelInit();
	TimerCycles = 0;
	SchedulerCreateThread(
			&SleeperThread,
			1,
			SleeperStack,
			STACK_SIZE,
			SleeperMain,
			NULL,
			2,
			TRUE);

	WorkerCreateWorker(
			&WorkerThread,
			WorkerStack,
			STACK_SIZE,
			3);

	KernelStart();
	return 0;
}
