#include"timer.h"
#include"hal.h"
#include"scheduler.h"
#include"interrupt.h"

void KernelInit()
{//entry point for kernel...
	HalStartup();

	InterruptStartup();

	TimerStartup();

	SchedulerStartup();
}

void KernelStart()
{
	InterruptEnable();
	while(1);
}

