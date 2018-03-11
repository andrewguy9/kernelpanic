#include"isr.h"
#include"hal.h"

/*
 * Isr Unit Description
 * The Isr unit provides counted interface for controlling the current
 * IRQ_LEVEL.
 *
 * Calls to IsrDisable/IsrEnable allow functions to nest
 * disable/enable pairings so that we don't have to track all code paths 
 * around flag state changes.
 *
 * IsrDisabledCount[level] is a count used to track the number of disables, or in other
 * words, the number of enables before interrupts of that level will be allowed.
 * When IsrDisabledCount[level] == 0, then ISRs should be allowed.
 * When IsrDisabledCount[level] > 0, then ISR will be disabled.
 */

//
//ISR Variables
//

volatile COUNT IsrDisabledCount[IRQ_LEVEL_COUNT];
ISR_HANDLER* IsrHandlerTable[IRQ_LEVEL_COUNT];

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void IsrStartup()
{
        INDEX i;
        for(i = 0; i < IRQ_LEVEL_COUNT; i++) {
                IsrDisabledCount[i] = 0;
                IsrHandlerTable[i] = NULL;
        }
}

//
//Register new ISR
//

void IsrHandlerWrapper(enum IRQ_LEVEL level) {
        IsrDisable(level);
        IsrHandlerTable[level]();
        IsrEnable(level);
}

void IsrRegisterHandler( ISR_HANDLER handler, void * which, enum IRQ_LEVEL level)
{
        IsrHandlerTable[level] = handler;
        HalRegisterIsrHandler( IsrHandlerWrapper, which, level );
}

//
//Handle Atomic Sections
//

/*
 * Used to turn off Interrupts. Can be called recursively.
 */
void IsrDisable(enum IRQ_LEVEL level)
{
        if( IsrDisabledCount[level] == 0 ) {
                IsrDefer( level, FALSE );
        }

        IsrDisabledCount[level]++;
}

/*
 * Used to turn Interrupts back on. Can be called recursively.
 */
void IsrEnable(enum IRQ_LEVEL level)
{
        ASSERT( HalIsIrqAtomic( level ) );
        ASSERT( IsrDisabledCount[level] > 0 );

        IsrDisabledCount[level]--;

        if( IsrDisabledCount[level] == 0 ) {
                //Because we are on the falling edge of a counted
                //IsrDisable call we will need to change the
                //interrupt mask. We should call IsrDefer
                //so that the correct mask can be selected.
                IsrDefer( level, TRUE );
        }
}

void IsrIncrement(enum IRQ_LEVEL level)
{
        ASSERT( HalIsIrqAtomic( level ) );
        ASSERT( IsrDisabledCount[level] == 0 );

        IsrDisabledCount[level]++;
}

void IsrDecrement(enum IRQ_LEVEL level)
{
        ASSERT( HalIsIrqAtomic( level ) );
        ASSERT( IsrDisabledCount[level] == 1 );

        //We trust that the stack under us will restore the proper interrupt levels.
        IsrDisabledCount[level]--;
}

//
//Functions for Sanity Checking
//

#ifdef DEBUG
/*
 * Should be called only by assertions as this
 * is not gauranteed to produce accurate results.
 */
BOOL IsrIsAtomic(enum IRQ_LEVEL level)
{
        //
        //If HalIsIrqAtomic(level) for a level is true,
        //then the IsrDisabledCount[] should be positive for levels 'level' to IRQ_LEVEL_COUNT.
        //since we are physically atomic.
        //If HalIsIrqAtomic(level) for a level is false,
        //then interrupt level should be 0, because we have
        //interrupts enabled.
        //

        ASSERT( level != IRQ_LEVEL_NONE);//Because enums can be signed or unsigned we need to make sure IRQ_LEVEL_NONE aka 0 is never passed.

        enum IRQ_LEVEL l;
        for(l = IRQ_LEVEL_COUNT - 1; l >= level; l--) {
                if( IsrDisabledCount[l] == 0 ) {
                        ASSERT( ! HalIsIrqAtomic( l ) );
                } else {
                        //Some level higher than level is atomic.
                        //That means that level itself is atomic.
                        ASSERT( HalIsIrqAtomic( l ) );
                        return TRUE;
                }
        }
        return FALSE;
}

/*
 * Should be called only by assertions at top and bottom
 * of gcs.
 */
BOOL IsrIsEdge(enum IRQ_LEVEL level)
{
        if( HalIsIrqAtomic( level ) && IsrDisabledCount[level] == 0 ) {
                return TRUE;
        } else {
                return FALSE;
        }
}
#endif //DEBUG

BOOL IsrCheckLevel( enum IRQ_LEVEL changingLevel, enum IRQ_LEVEL candidateLevel, BOOL enable)
{
        if( IsrDisabledCount[candidateLevel] > 0 || (candidateLevel == changingLevel && ! enable) ) {
                HalSetIrq(candidateLevel);
                return TRUE;
        } else {
                return FALSE;
        }
}

/*
 * Called by the Interrupt, SoftInterrupt and CritInterrupt
 * units when re-enabling interrupts. This allows for selection
 * of the highest priority mask.
 *
 * level is the level being changed.
 * enable is true if we are lowering the disabled count.
 * enable is false if we are raising the disabled count.
 */
void IsrDefer( enum IRQ_LEVEL level, BOOL enable )
{
        enum IRQ_LEVEL l;
        for(l = IRQ_LEVEL_COUNT - 1; l > IRQ_LEVEL_NONE; l--) {
                if( IsrCheckLevel(level, l, enable) ) {
                        return;
                }
        }

        //If we get here, then we know that our IRQ should be passive.
        HalSetIrq(IRQ_LEVEL_NONE);
}


