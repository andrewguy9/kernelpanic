#include"timer.h"
#include"hal.h"
#include"scheduler.h"
#include"timer.h"
#include"interrupt.h"
#include"context.h"
#include"panic.h"

/*
 * Startup Unit:
 * The startup unit bootstraps the kernel. 
 */

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
 * Called to complete start up. This function
 * never returns.
 */
void KernelStart()
{
	InterruptEnable();
	while(1);
	KernelPanic();
}

