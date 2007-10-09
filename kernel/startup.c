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
	HalEnableInterrupts();
}

int main()
{
	KernelInit();
	KernelStart();
	return 0;
}
