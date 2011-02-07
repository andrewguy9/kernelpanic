#include"softinterrupt.h"
#include"interrupt.h"
#include"critinterrupt.h"
#include"../utils/linkedlist.h"
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
struct LINKED_LIST SoftInterruptHandlerList;

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void SoftInterruptStartup()
{
	ASSERT( HalIsSoftAtomic() );

	SoftInterruptLevel = 1;//Will be reset to 0 when startup completes
	LinkedListInit( &SoftInterruptHandlerList );
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
		InterruptDefer( IRQ_LEVEL_SOFT, FALSE );
	}

	SoftInterruptLevel++;
}

/*
 * Called by threads or PostInterruptHandlers to turn
 * SoftInterrupts back on. Can be called recirsivly. 
 */
void SoftInterruptEnable()
{
	ASSERT( HalIsSoftAtomic() );
	ASSERT( SoftInterruptLevel > 0 );

	SoftInterruptLevel--;

	if( SoftInterruptLevel == 0 )
	{
		//Because we are on the falling edge of a counted 
		//SoftInterruptDisable call we will need to change the
		//interrupt mask. We should call InterruptDefer 
		//so that the correct mask can be selected.
		//NOTE: Even though this is the soft unit we still must 
		//call InterruptDefer (Because we still may need a full
		//interrupt mask).
		InterruptDefer( IRQ_LEVEL_SOFT, TRUE );
	}
}

void SoftInterruptIncrement()
{
	ASSERT( HalIsSoftAtomic() );
	ASSERT( SoftInterruptLevel == 0 );

	SoftInterruptLevel++;
}

void SoftInterruptDecrement()
{
	ASSERT( HalIsSoftAtomic() );
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

		ASSERT( ! HalIsSoftAtomic() );
		return FALSE;
	}
	else 
	{
		ASSERT( HalIsSoftAtomic() );
		return TRUE;
	}
}

/*
 * Should be called only by assertions at top and bottom
 * of ISRs.
 */
BOOL SoftInterruptIsEdge()
{
	if( HalIsSoftAtomic() && SoftInterruptLevel == 0 )
		return TRUE;
	else 
		return FALSE;
}
#endif //DEBUG


void SoftInterrupt()
{
	struct HANDLER_OBJECT * handler;
	BOOL isComplete;
	HANDLER_FUNCTION * func;

	SoftInterruptIncrement();

	InterruptDisable();
	while( ! LinkedListIsEmpty( & SoftInterruptHandlerList ) )
	{
		handler = BASE_OBJECT(
				LinkedListPop( & SoftInterruptHandlerList ),
				struct HANDLER_OBJECT,
				Link );
		
		InterruptEnable();

		HandlerRun( handler );
		func = handler->Function;
		isComplete = func( handler );

		if(isComplete) 
		{
			HandlerFinish( handler );
		}

		InterruptDisable();
	}
	InterruptEnable();

	SoftInterruptDecrement();
}

void SoftInterruptRegisterHandler(
		struct HANDLER_OBJECT * handler,
		HANDLER_FUNCTION foo,
		void * context )
{

	handler->Function = foo;
	handler->Context = context;

	HandlerRegister( handler );

	InterruptDisable();
	LinkedListEnqueue( &handler->Link.LinkedListLink,
			& SoftInterruptHandlerList );
	InterruptEnable();

	HalRaiseSoftInterrupt();
}

/*
 * Called by the Interrupt unit when he determines that he 
 * does not know what interrupt mask to apply. 
 * If SoftInterrupts are disabled we will apply that mask,
 * otherwise we defer to the Crit unit.
 */
void SoftInterruptDefer( enum IRQ_LEVEL level, BOOL enable )
{
	if( SoftInterruptLevel > 0 || ( level == IRQ_LEVEL_SOFT  && ! enable ) )
	{
		//Soft Interrupts are disabled, so we should set the 
		//soft disabled mask.
		HalSetIrq(IRQ_LEVEL_SOFT);
	}
	else
	{
		//Soft interrupts are allowed, we should defer to crit interrupts.
		CritInterruptDefer( level, enable );
	}
}

