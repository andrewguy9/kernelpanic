#include"softinterrupt.h"
#include"hal.h"

/*
 * SoftInterrupt Unit Description
 * The SoftInterrupt unit provides additional control of the SoftInterrupt flag.
 *
 * Calls to SoftInterruptDisable/SoftInterruptEnable allow functions to nest 
 * disable/enable pairings so that we don't have to track all code paths 
 * around flag state changes.
 *
 * SoftInterruptLevel is a count used to track the number of disables, or in other
 * words, the number of enables before SoftInterrupts will be allowed.
 * When SoftInterruptLevel == 0, then interrupts should be allowed.
 * When SoftInterruptLevel > 0, then SoftInterrupt will be disabled.
 *
 */

//
//SoftInterrupt Variables
//

volatile COUNT SoftInterruptLevel;//The number of calls to SoftInterruptDisable

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void SoftInterruptStartup()
{
	ASSERT( HalIsSoftAtomic() );

	SoftInterruptLevel = 1;//Will be reset to 0 when startup completes
}

//
//Handle Atomic Sections
//

/*
 * Used by threads or PostInterruptHandlers to turn 
 * off SoftInterrupts. Can be called recursively.
 */
void SoftInterruptDisable()
{
	if( SoftInterruptLevel == 0 ) 
	{
		HalDisableSoftInterrupts();
	}

	SoftInterruptLevel++;
}

/*
 * Calleed by threads or PostInterruptHandlers to turn
 * SoftInterrupts back on. Can be called recirsivly. 
 */
void SoftInterruptEnable()
{
	ASSERT( HalIsAtomic() );
	ASSERT( SoftInterruptLevel > 0 );

	SoftInterruptLevel--;

	if( SoftInterruptLevel == 0 )
	{
		HalEnableSoftInterrupts();
	}
}

void SoftInterruptIncrement()
{
	ASSERT( HalIsAtomic() );
	ASSERT( SoftInterruptLevel == 0 );

	SoftInterruptLevel++;
}

void SoftInterruptDecrement()
{
	ASSERT( HalIsAtomic() );
	ASSERT( SoftInterruptLevel == 1 );

	SoftInterruptLevel--;
}

//
//Functions for Sanity Checking
//

#ifdef DEBUG
/*
 * Should be called only by assertions as this
 * is not gauranteed to produce accurate results.
 */
BOOL SoftInterruptIsAtomic()
{
	//
	//If HalIsAtomic is true, 
	//then the SoftInterruptLevel should be positive,
	//since we are physically atomic.
	//If HalIsAtomic is false,
	//then SoftInterrupt level should be 0, because we have 
	//SoftInterrupts enabled.
	//

	if( SoftInterruptLevel == 0 )
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
BOOL SoftInterruptIsEdge()
{
	if( HalIsAtomic() && SoftInterruptLevel == 0 )
		return TRUE;
	else 
		return FALSE;
}
#endif //DEBUG

