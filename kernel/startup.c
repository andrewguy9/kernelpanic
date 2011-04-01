#include"hal.h"
#include"isr.h"
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
	HalIsrInit();

	IsrStartup();
	CritInterruptStartup();
	SoftInterruptStartup();

	WatchdogStartup();

	TimerStartup();

	//TODO THIS IS A STARTUP HACK, WE SHOULD FIX IT.
	IsrDisable(IRQ_LEVEL_TIMER);

	SchedulerStartup();
	
}

/*
 * Called to complete start up. This function
 * never returns.
 */
void KernelStart()
{
	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_TIMER) );
	//TODO THIS IS A STARTUP HACK, WE SHOULD FIX IT.
	IsrEnable(IRQ_LEVEL_TIMER);
	ASSERT( ! HalIsIrqAtomic(IRQ_LEVEL_TIMER) );

	while(1)
	{
		HalSleepProcessor();
	}

	KernelPanic();
}

