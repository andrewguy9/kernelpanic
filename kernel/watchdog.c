#include"watchdog.h"
#include"hal.h"
#include"panic.h"
#include"interrupt.h"
#include"../utils/bitfield.h"

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

BITFIELD WatchdogDesiredMask;

#ifdef DEBUG 
volatile TIME WatchdogLastUpdatedTime;
volatile TIME WatchdogLastClearedTime;
#endif

/*
 * Call this to setup the watchdog system
 */
void WatchdogStartup( )
{
	WatchdogDesiredMask = FLAG_NONE;
#ifdef DEBUG 
	WatchdogLastUpdatedTime = 0;
	WatchdogLastClearedTime = 0;
#endif
}

void WatchdogEnable( int frequency )
{
	HalEnableWatchdog( frequency );
}

/*
 * Call this routine to tell the watchdog that the flag
 * owner has run.
 */
void WatchdogNotify( INDEX index )
{
	BITMAP_WORD flag;
#ifdef DEBUG
	TIME time;
#endif

	//We ignore index 0.
	if( index == 0 )
	{
		return;
	}
	else
	{
		index--;
	}

	//Ensure that we are using a valid flag.
	ASSERT( index <= FLAG_MAX_INDEX );

	//find the bit we want to flip.
	flag = FlagGetBit( index );
	
	InterruptDisable();

	//Assert that this flag is present in the desired mask.
	ASSERT(FlagGet(flag, WatchdogDesiredMask));

#ifdef DEBUG
	time = TimerGetTime();
	WatchdogLastUpdatedTime = time;
#endif
	//TODO HAVING THE WATCH DOG MASK IN THE HAL BREAKS LAYERING
	FlagOn( HalWatchdogMask, flag );
	//check to see if all of the players have shown up.
	if( FlagsEqual(HalWatchdogMask, WatchdogDesiredMask) )
	{
		//We have flipped all the flags required.
		//So lets pet the watchdog.
		HalPetWatchdog();
		//Nhow lets clear the mask because we need to
		//restart our checking.
		HalWatchdogMask = FLAG_NONE;

#ifdef DEBUG
		WatchdogLastClearedTime = time;
#endif
	}
	InterruptEnable();
}

void WatchdogAddFlag( INDEX index )
{
	BITFIELD flag = FLAG_NONE;

	if( index == 0 )
	{
		return;
	}
	else
	{
		//We shift index down because we start numbering bits at 1.
		//zero should be ignored.
		index--;
	}

	ASSERT( index <= FLAG_MAX_INDEX );

	flag = FlagGetBit( index );

	InterruptDisable();
	FlagOn( WatchdogDesiredMask, flag );
	InterruptEnable();
}
