#include"timer.h"
#include"scheduler.h"
#include"hal.h"

void KernelInit()
{//entry point for kernel...
	HalInit();

	TimerInit( );

	SchedulerInit();

}

void KernelStart()
{
	TIME lastTime=0xffff;
	TIME curTime;
	HalEnableInterrupts();
	while(1)
	{
		do
		{
			curTime = TimerGetTime();
		}while( curTime == lastTime );
		lastTime = curTime;
	}
}

