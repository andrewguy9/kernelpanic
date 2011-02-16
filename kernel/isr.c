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

volatile COUNT IsrDisabledCount[IRQ_LEVEL_MAX + 1];

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void IsrStartup()
{
	INDEX i;
	for(i = 0; i < IRQ_LEVEL_MAX + 1; i++) {
		IsrDisabledCount[i] = 0;
	}
	//TODO THIS IS A HACK. Should be separate call.
	//We need to make this contract explicit.
	ASSERT( HalIsIrqAtomic( IRQ_LEVEL_TIMER ) );
	IsrDisabledCount[IRQ_LEVEL_TIMER] = 1;
}

//
//Register new ISR
//

void IsrRegisterHandler( ISR_HANDLER handler, void * which, enum IRQ_LEVEL level)
{
	HalRegisterIsrHandler( handler, which, level );
}

//
//Handle Atomic Sections
//

/*
 * Used to turn off Interrupts. Can be called recursively.
 */
void IsrDisable(enum IRQ_LEVEL level)
{
	if( IsrDisabledCount[level] == 0 ) 
	{
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

	if( IsrDisabledCount[level] == 0 )
	{
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
	//then the IsrDisabledCount[level] should be positive,
	//since we are physically atomic.
	//If HalIsIrqAtomic(level) for a level is false,
	//then interrupt level should be 0, because we have 
	//interrupts enabled.
	//

	if( IsrDisabledCount[level] == 0 )
	{

		ASSERT( ! HalIsIrqAtomic( level ) );
		return FALSE;
	}
	else 
	{
		ASSERT( HalIsIrqAtomic( level ) );
		return TRUE;
	}
}

/*
 * Should be called only by assertions at top and bottom
 * of gcs.
 */
BOOL IsrIsEdge(enum IRQ_LEVEL level)
{
	if( HalIsIrqAtomic( level ) && IsrDisabledCount[level] == 0 )
		return TRUE;
	else 
		return FALSE;
}
#endif //DEBUG

BOOL IsrCheckLevel( enum IRQ_LEVEL changingLevel, enum IRQ_LEVEL candidateLevel, BOOL enable) 
{
	if( IsrDisabledCount[candidateLevel] > 0 || (candidateLevel == changingLevel && ! enable) ) 
	{
		HalSetIrq(candidateLevel);
		return TRUE;
	} 
	else 
	{
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
#if 0 

	if( IsrDisabledCount[level] > 0 || (level == IRQ_LEVEL_TIMER && ! enable ) )
	{
		//Interrupts are disabled, so we should set the 
		//interrupt disabled mask.
		HalSetIrq(IRQ_LEVEL_TIMER);
	}
	else
	{
		//interrupts are allowed, we should defer to crit interrupts.
		SoftIsrDefer( level, enable );
	}

#else 
	
	enum IRQ_LEVEL l;
	for(l = IRQ_LEVEL_MAX; l > IRQ_LEVEL_NONE; l--) {
		if( IsrCheckLevel(level, l, enable) ) {
			return;
		}
	}

	//If we get here, then we know that our IRQ should be passive.
	HalSetIrq(IRQ_LEVEL_NONE);
#endif
}


