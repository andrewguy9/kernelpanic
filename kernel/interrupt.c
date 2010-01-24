#include"interrupt.h"
#include"hal.h"
#include"softinterrupt.h"
#include"critinterrupt.h"

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
 * off Interrupts. Can be called recursively.
 */
void InterruptDisable()
{
	if( InterruptLevel++ == 0 ) 
	{
		InterruptDefer();
	}

}

/*
 * Called by threads or PostInterruptHandlers to turn
 * Interrupts back on. Can be called recirsivly. 
 */
void InterruptEnable()
{
	ASSERT( HalIsAtomic() );
	ASSERT( InterruptLevel > 0 );

	InterruptLevel--;

	if( InterruptLevel == 0 )
	{
		//Because we are on the falling edge of a counted 
		//InterruptDisable call we will need to change the
		//interrupt mask. We should call InterruptDefer 
		//so that the correct mask can be selected.
		InterruptDefer();
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

	//We trust that the stack under us will restore the proper interrupt levels.
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

/*
 * Called by the Interrupt, SoftInterrupt and CritInterrupt 
 * units when re-enabling interrupts. This allows for selection
 * of the highest priority mask.
 */
void InterruptDefer()
{
	if( InterruptLevel > 0 )
	{
		//Interrupts are disabled, so we should set the 
		//interrupt disabled mask.
		HalDisableInterrupts();
	}
	else
	{
		//interrupts are allowed, we should defer to crit interrupts.
		SoftInterruptDefer();
	}
}

