#include"timer.h"
#include"hal.h"
#include"scheduler.h"
#include"timer.h"
#include"interrupt.h"
#include"context.h"

/*
 * Initializes the kernel structures.
 * Should be called before any other kernel function.
 */
void KernelInit()
{//entry point for kernel...
	HalStartup();

	InterruptStartup();

	TimerStartup();

	SchedulerStartup();
}

/*
 * Called to complete start up.
 */
void KernelStart()
{
	InterruptEnable();
	while(1);
}

