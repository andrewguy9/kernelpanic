#include"watchdog.h"
#include"hal.h"
#include"panic.h"
#include"isr.h"
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

//
//Prototypes
//

void WatchdogInterrupt();

//
//Globals
//

BITFIELD WatchdogDesiredMask;
BITFIELD WatchdogCurMask;
TIME Timeout;

#ifdef DEBUG
TIME WatchdogLastUpdatedTime;
TIME WatchdogLastClearedTime;
#endif

/*
 * Call this to setup the watchdog system
 */
void WatchdogStartup( )
{
        WatchdogDesiredMask = FLAG_NONE;
        WatchdogCurMask = FLAG_NONE;

#ifdef DEBUG
        WatchdogLastUpdatedTime = 0;
        WatchdogLastClearedTime = 0;
#endif

        HalWatchdogInit();
        HalRegisterIsrHandler( WatchdogInterrupt, (void *) HAL_ISR_WATCHDOG, IRQ_LEVEL_WATCHDOG );
}

void WatchdogEnable( TIME timeout )
{
        Timeout = timeout;
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

        //Ensure that we are using a valid flag.
        ASSERT( index <= FLAG_MAX_INDEX );

        //find the bit we want to flip.
        flag = FlagGetBit( index );

        IsrDisable(IRQ_LEVEL_WATCHDOG);

        //Assert that this flag is present in the desired mask.
        ASSERT(FlagGet(flag, WatchdogDesiredMask));

#ifdef DEBUG
        time = TimerGetTime();
        WatchdogLastUpdatedTime = time;
#endif
        FlagOn( WatchdogCurMask, flag );
        //check to see if all of the players have shown up.
        if( FlagsEqual(WatchdogCurMask, WatchdogDesiredMask) )
        {
                //We have flipped all the flags required.
                //So lets pet the watchdog.
                HalPetWatchdog(Timeout);
                //Now lets clear the mask because we need to
                //restart our checking.
                WatchdogCurMask = FLAG_NONE;

#ifdef DEBUG
                WatchdogLastClearedTime = time;
#endif
        }
        IsrEnable(IRQ_LEVEL_WATCHDOG);
}

void WatchdogAddFlag( INDEX index )
{
        BITFIELD flag = FLAG_NONE;

        ASSERT( index <= FLAG_MAX_INDEX );

        flag = FlagGetBit( index );

        IsrDisable(IRQ_LEVEL_WATCHDOG);
        FlagOn( WatchdogDesiredMask, flag );
        IsrEnable(IRQ_LEVEL_WATCHDOG);
}

void WatchdogInterrupt()
{
        KernelPanic();
}
