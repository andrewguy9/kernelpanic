#include"hal.h"
#include"interrupt.h"
#include"timer.h"
#include"scheduler.h"
#include"panic.h"
#include"watchdog.h"

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

	WatchdogStartup();

	TimerStartup();

	SchedulerStartup();
}

/*
 * Called to complete start up. This function
 * never returns.
 */
void KernelStart()
{
	ASSERT( HalIsAtomic() );
	InterruptEnable();
	ASSERT( ! HalIsAtomic() );

	while(1)
	{
		HalSleepProcessor();
	}

	KernelPanic();
}

