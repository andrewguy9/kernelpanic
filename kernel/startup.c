#include"timer.h"
#include"hal.h"
#include"scheduler.h"
#include"interrupt.h"

void KernelInit()
{//entry point for kernel...
	HalStartup();

	TimerStartup();

	SchedulerStartup();

}

void KernelStart()
{
	TIME lastTime=0xffff;
	TIME curTime;
	InterruptEnable();
	while(1)
	{
		do
		{
			curTime = TimerGetTime();
		}while( curTime == lastTime );
		lastTime = curTime;
	}
}

