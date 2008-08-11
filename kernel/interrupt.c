#include"interrupt.h"
#include"hal.h"

/*
 * Interrupt Unit Description
 * The interrupt unit provides additional control to the interrupt flag.
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
 * Inorder to reduce the time in which interrupts are disabled,
 * the interrupt unit allows for "post interrupt callbacks" 
 * which can be scheduled using InterruptRegisterPostHandler( ).
 * 
 * When exiting an atomic section (as intterupt level goes from 1 to 0), 
 * we reenable interrupts and run 
 * the post interrupt handlers with the interrupts off and then return.
 * This allows the heavy lifting in interrupts to be done with interrupts
 * enabled. The benefit is really short atomic sections, by running heavy
 * operations in post interrupt handlers.
 *
 * To prevent infinite interrupt nesting we use the only allow the bottom
 * interrupt on the stack to process post handlers.
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
	HalDisableInterrupts();
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
	ASSERT( HalIsAtomic() && InterruptLevel == 0 );

	InterruptLevel++;
}

void InterruptDecrement()
{
	ASSERT( HalIsAtomic() && InterruptLevel == 1 );

	InterruptLevel--;
}
//
//Functions for Sanity Checking
//

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
		return FALSE;
	else 
		return TRUE;
}
