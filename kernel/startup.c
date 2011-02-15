#include"hal.h"
#include"interrupt.h"
#include"softinterrupt.h"
#include"critinterrupt.h"
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

	IsrStartup();
	SoftInterruptStartup();
	CritInterruptStartup();

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
	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );
	InterruptEnable();
	ASSERT( ! HalIsIrqAtomic(IRQ_LEVEL_TIMER) );

	while(1)
	{
		HalSleepProcessor();
	}

	KernelPanic();
}

