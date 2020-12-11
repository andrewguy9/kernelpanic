#include"kernel/sleep.h"
#include"kernel/scheduler.h"
#include"kernel/timer.h"
#include"utils/utils.h"
#include"kernel/startup.h"
#include"kernel/panic.h"
#include"kernel/critinterrupt.h"

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

volatile COUNT TimerCycles;//Times we have run the test.

//Crit Function
HANDLER_FUNCTION CritHandler;
_Bool CritHandler( struct HANDLER_OBJECT * handler )
{
	ASSERT( SchedulerIsCritical() );
	if( TimerCycles != (COUNT) handler->Context ) {
		KernelPanic( );
	}

	return true;
}

//Timer Function
HANDLER_FUNCTION TimerHandler;
_Bool TimerHandler( struct HANDLER_OBJECT * handler )
{
	CritInterruptRegisterHandler(
			handler,
			CritHandler,
			handler->Context);

	return false;
}

//Thread Main
THREAD_MAIN SleeperMain;
void * SleeperMain(void * unused)
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
					(void *) TimerCycles);

			//Go to sleep:
			Sleep( Sequence[cur] );

			//Increase our iteration count.
			TimerCycles++;

			//Check to see if the timer fired before we woke.
			IsrDisable(IRQ_LEVEL_MAX);
			if( !HandlerIsFinished( &Timer ) )
			{
				KernelPanic( );
			}
			IsrEnable(IRQ_LEVEL_MAX);
		}
	}
        return NULL;
}

int main()
{
        KernelInit();

        SchedulerStartup();

        TimerCycles = 0;

        HandlerInit( &Timer );

        SchedulerCreateThread(
                        &SleeperThread,
                        1,
                        SleeperStack,
                        STACK_SIZE,
                        SleeperMain,
                        NULL,
                        NULL,
                        true);

        KernelStart();
        return 0;
}
