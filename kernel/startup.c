#include"hal.h"
#include"isr.h"
#include"softinterrupt.h"
#include"critinterrupt.h"
#include"timer.h"
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
        //TODO THIS IS A STARTUP HACK, WE SHOULD FIX IT.
        IsrDisable(IRQ_LEVEL_MAX);

        CritInterruptStartup();
        SoftInterruptStartup();

        TimerStartup();//TODO THE TIMER REGISTERED IN THIS FUNCTION IS RACING THE CALL WE MAKE TO IsrDisable.
}

/*
 * Called to complete start up. This function
 * never returns.
 */
void KernelStart()
{
        ASSERT( HalIsIrqAtomic(IRQ_LEVEL_MAX) );
        //TODO THIS IS A STARTUP HACK, WE SHOULD FIX IT.
        IsrEnable(IRQ_LEVEL_MAX);
        ASSERT( ! HalIsIrqAtomic(IRQ_LEVEL_MAX) );

        while(1) {
                HalSleepProcessor();
        }

        KernelPanic();
}

