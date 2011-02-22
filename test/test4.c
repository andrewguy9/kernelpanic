#include"../kernel/sleep.h"
#include"../kernel/scheduler.h"
#include"../kernel/timer.h"
#include"../utils/utils.h"
#include"../kernel/startup.h"
#include"../kernel/interrupt.h"
#include"../kernel/panic.h"
#include"../kernel/critinterrupt.h"

/*
 * Tests the sleep unit, should panic on failure.
 */

//Define Sleep Patern
#define SEQUENCE_LENGTH 8
COUNT Sequence[SEQUENCE_LENGTH] = {4,8,16,32,64,128,256,512};

//Define Thread
struct THREAD SleeperThread;

#define STACK_SIZE HAL_MIN_STACK_SIZE

char SleeperStack[STACK_SIZE];

//Define Timer
struct HANDLER_OBJECT Timer;

COUNT TimerCycles;//Times we have run the test.

//Crit Function
BOOL CritHandler( struct HANDLER_OBJECT * handler )
{
	ASSERT( SchedulerIsCritical() );
	if( !SchedulerIsThreadBlocked( &SleeperThread ) ) {
		KernelPanic( );
	}

	return TRUE;
}

//Timer Function
BOOL TimerHandler( struct HANDLER_OBJECT * handler )
{
	CritInterruptRegisterHandler(
			handler,
			CritHandler,
			NULL );

	return FALSE;
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
			IsrDisable(IRQ_LEVEL_MAX);
			if( !HandlerIsFinished( &Timer ) )
			{
				KernelPanic( );
			}
			IsrEnable(IRQ_LEVEL_MAX);

			//Increase our iteration count.
			TimerCycles++;
		}
	}
}

int main()
{
	KernelInit();
	TimerCycles = 0;

	HandlerInit( &Timer );

	SchedulerCreateThread(
			&SleeperThread,
			1,
			SleeperStack,
			STACK_SIZE,
			SleeperMain,
			NULL,
			2,
			TRUE);

	KernelStart();
	return 0;
}
