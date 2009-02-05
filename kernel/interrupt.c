#include"interrupt.h"
#include"hal.h"

/*
 * Interrupt Unit Description
 * The interrupt unit provides additional control of the interrupt flag.
 *
 * Calls to InterruptDisable/InterruptEnable allow functions to nest 
 * disable/enable pairings so that we don't have to track all code paths 
 * around flag state changes.
 *
 * InterruptLevel is a count used to track the number of disables, or in other
 * words, the number of enables before interrupts will be allowed.
 * When InterruptLevel == 0, then interrupts should be allowed.
 * When InterruptLevel > 0, then interrupt will be disabled.
 *
 */

//
//Interrupt Variables
//

volatile COUNT InterruptLevel;//The number of calls to InterruptDisable

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void InterruptStartup()
{
	ASSERT( HalIsAtomic() );

	InterruptLevel = 1;//Will be reset to 0 when startup completes
}

//
//Handle Atomic Sections
//

/*
 * Used by threads or PostInterruptHandlers to turn 
 * off interrupts. Can be called recursively.
 */
void InterruptDisable()
{
	if( InterruptLevel == 0 ) 
	{
		HalDisableInterrupts();
	}

	InterruptLevel++;
}

/*
 * Calleed by threads or PostInterruptHandlers to turn
 * interrupts back on. Can be called recirsivly. 
 */
void InterruptEnable()
{
	ASSERT( HalIsAtomic() );
	ASSERT( InterruptLevel > 0 );

	InterruptLevel--;

	if( InterruptLevel == 0 )
	{
		HalEnableInterrupts();
	}
}

void InterruptIncrement()
{
	ASSERT( HalIsAtomic() );
	ASSERT( InterruptLevel == 0 );

	InterruptLevel++;
}

void InterruptDecrement()
{
	ASSERT( HalIsAtomic() );
	ASSERT( InterruptLevel == 1 );

	InterruptLevel--;
}

//
//Functions for Sanity Checking
//

#ifdef DEBUG
/*
 * Should be called only by assertions as this
 * is not gauranteed to produce accurate results.
 */
BOOL InterruptIsAtomic()
{
	//
	//If HalIsAtomic is true, 
	//then the InterruptLevel should be positive,
	//since we are physically atomic.
	//If HalIsAtomic is false,
	//then interrupt level should be 0, because we have 
	//interrupts enabled.
	//

	if( InterruptLevel == 0 )
	{

		ASSERT( ! HalIsAtomic() );
		return FALSE;
	}
	else 
	{
		ASSERT( HalIsAtomic() );
		return TRUE;
	}
}

/*
 * Should be called only by assertions at top and bottom
 * of ISRs.
 */
BOOL InterruptIsEdge()
{
	if( HalIsAtomic() && InterruptLevel == 0 )
		return TRUE;
	else 
		return FALSE;
}
#endif //DEBUG

