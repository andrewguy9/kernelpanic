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

#define STACK_SIZE HAL_MIN_STACK_SIZE

char SleeperStack[STACK_SIZE];
char WorkerStack[STACK_SIZE];

//Define Timer
struct HANDLER_OBJECT Timer;

COUNT TimerCycles;//Times we have run the test.

//Timer Function
BOOL TimerHandler( struct HANDLER_OBJECT * handler )
{
	//Opportunistically lock the thread structures.
	//If we can't get it we should not validate.
	if( ContextLock() ) {
		if( !SchedulerIsThreadBlocked( &SleeperThread ) ) {
			KernelPanic( );
		}
		ContextUnlock();
	}

	return TRUE;
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
			TimerRegister(
					& Timer,
					Sequence[cur] - 1,
					TimerHandler,
					NULL);

			//Go to sleep:
			Sleep( Sequence[cur] );

			//Check to see if the timer fired before we woke.
			InterruptDisable();
			if( !HandlerIsFinished( &Timer ) )
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

	TimerInit( &Timer );

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
