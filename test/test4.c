#include"../kernel/sleep.h"
#include"../kernel/scheduler.h"
#include"../kernel/timer.h"
#include"../utils/utils.h"
#include"../kernel/startup.h"

//Define Sleep Patern
#define SEQUENCE_LENGTH 8
COUNT Sequence[SEQUENCE_LENGTH] = {2,4,8,16,32,64,128,256};

//Define Thread
struct THREAD SleeperThread;

#define STACK_SIZE 300
char SleeperStack[STACK_SIZE];

//Define Timer
struct HANDLER_OBJECT Timer;

//Define Global Flags
BOOL TimerFlag;
BOOL ThreadFlag;

//TimerFunction
void TimerHandler( void * Argument )
{
	//Clear Flag
	TimerFlag = FALSE;
	//Check to see if thread is sleeping
	if( ThreadFlag == FALSE )
		KernelPanic( 0 );
}

//Thread Main
void SleeperMain()
{
	INDEX cur=0;
	while(1)
	{
		//Register Timer
		InterruptDisable();
		TimerFlag = TRUE;
		InterruptEnable();

		TimerRegister(
				& Timer,
				Sequence[cur] - 1,
				TimerHandler,
				NULL);

		//Go to sleep
		InterruptDisable();
		ThreadFlag = TRUE;
		InterruptEnable();

		Sleep( Sequence[cur] );

		InterruptDisable();
		ThreadFlag = FALSE;
		InterruptEnable();

		//Check to see timer fired
		InterruptDisable();
		if( TimerFlag != FALSE )
		{
			KernelPanic( 0 );
		}
		InterruptEnable();

		//Move to next sequence
		cur = cur+1 % SEQUENCE_LENGTH;
	}
}

int main()
{
	KernelInit();
	SchedulerCreateThread(
			&SleeperThread,
			1,
			SleeperStack,
			STACK_SIZE,
			SleeperMain);
	KernelStart();
	return 0;
}
