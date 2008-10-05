#include"watchdog.h"
#include"hal.h"
#include"panic.h"
#include"interrupt.h"

/*
 * Watchdog System:
 * The watchdog unit exposes a watch dog system which makes sure that all elements
 * in the system are running (not stalled).
 * The system has two 8 bit flags, the HalRunningMask and the WatchdogDesiredMask.
 * The HalRunningMask is used to figure out which components have run.
 * For instance, when a thread is run, its mask is applied to the running
 * mask. ISRs can also apply a flag to the mask.
 *
 * Then the watchdog timer expires the HalDesiredMask is compared against the 
 * HalRunningMask. If they dont match, then a component of the kernel didn't
 * run between runs of the watchdog. This is considered an error and the
 * kernel will panic. 
 *
 * It is the responsibility of the app developer to set the desired mask.
 * If a bit in the desired mask is 0, there will be no check on that bit.
 *
 * The HalRunningMask is defined in the hal so that it can be easily applied
 * to indicator lights on various platforms.
 */

char WatchdogDesiredMask;

/*
 * Call this to setup the watchdog system
 */
void WatchdogStartup(char desiredMask, int frequency)
{
	WatchdogDesiredMask = desiredMask;
	HalStartupWatchdog( frequency );
}

/*
 * Call this routine to tell the watchdog that the flag
 * owner has run.
 */
void WatchdogNotify( INDEX index )
{
	InterruptDisable();
	FlagOn( &(HAL_WATCHDOG_MASK),(index));
	if( (HAL_WATCHDOG_MASK ^ WatchdogDesiredMask) == 0 )
	{
		//We have flipped all the flags required.
		//So lets pet the watchdog.
		HalPetWatchdog();
		//Now lets clear the mask because we need to
		//restart our checking.
		HAL_WATCHDOG_MASK=0;
	}
	InterruptEnable();
}

