#include"critinterrupt.h"
#include"interrupt.h"
#include"hal.h"

/*
 * Crit Interrupt Unit Description
 * The interrupt unit provides additional control of the interrupt flag.
 *
 * Calls to CritInterruptDisable/CritInterruptEnable allow functions to nest 
 * disable/enable pairings so that we don't have to track all code paths 
 * around flag state changes.
 *
 * CritInterruptLevel is a count used to track the number of disables, or in other
 * words, the number of enables before crit interrupts will be allowed.
 * When CritInterruptLevel == 0, then crit interrupts should be allowed.
 * When CritInterruptLevel > 0, then crit interrupts will be disabled.
 *
 */

//
//Crit Interrupt Variables
//

volatile COUNT CritInterruptLevel;//The number of calls to CritInterruptDisable
struct LINKED_LIST CritInterruptHandlerList;

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void CritInterruptStartup()
{
	ASSERT( HalIsCritAtomic() );

	CritInterruptLevel = 1;//Will be reset to 0 when startup completes
	LinkedListInit( & CritInterruptHandlerList );
}

//
//Handle Atomic Sections
//

/*
 * Used by threads or PostInterruptHandlers to turn 
 * off Critinterrupts. Can be called recursively.
 */
void CritInterruptDisable()
{
	if( CritInterruptLevel++ == 0 ) 
	{
		InterruptDefer();
	}
}

/*
 * Called by threads or PostInterruptHandlers to turn
 * CritInterrupts back on. Can be called recirsivly. 
 */
void CritInterruptEnable()
{
	ASSERT( HalIsCritAtomic() );
	ASSERT( CritInterruptLevel > 0 );

	CritInterruptLevel--;

	if( CritInterruptLevel == 0 )
	{
		//Because we are on the falling edge of a counted 
		//CritInterruptDisable call we will need to change the
		//interrupt mask. We should call InterruptDefer 
		//so that the correct mask can be selected.
		//NOTE: Even though this is the crit unit we still must 
		//call InterruptDefer (Because we still may need a full
		//interrupt mask).
		InterruptDefer();
	}
}

void CritInterruptIncrement()
{
	ASSERT( HalIsCritAtomic() );
	ASSERT( CritInterruptLevel == 0 );

	CritInterruptLevel++;
}

void CritInterruptDecrement()
{
	ASSERT( HalIsCritAtomic() );
	ASSERT( CritInterruptLevel == 1 );

	CritInterruptLevel--;
}

//
//Functions for Sanity Checking
//

#ifdef DEBUG
/*
 * Should be called only by assertions as this
 * is not gauranteed to produce accurate results.
 */
BOOL CritInterruptIsAtomic()
{
	//
	//If HalIsAtomic is true, 
	//then the CritInterruptLevel should be positive,
	//since we are physically atomic.
	//If HalIsAtomic is false,
	//then Critinterrupt level should be 0, because we have 
	//CritInterrupts enabled.
	//

	if( CritInterruptLevel == 0 )
	{

		ASSERT( ! HalIsCritAtomic() );
		return FALSE;
	}
	else 
	{
		ASSERT( HalIsCritAtomic() );
		return TRUE;
	}
}

/*
 * Should be called only by assertions at top and bottom
 * of ISRs.
 */
BOOL CritInterruptIsEdge()
{
	if( HalIsCritAtomic() && CritInterruptLevel == 0 )
		return TRUE;
	else 
		return FALSE;
}
#endif //DEBUG


void CritInterrupt() 
{
	struct HANDLER_OBJECT * handler;
	BOOL isComplete;
	HANDLER_FUNCTION * func;

	CritInterruptIncrement();

	InterruptDisable();
	while( ! LinkedListIsEmpty( & CritInterruptHandlerList ) )
	{
		handler = BASE_OBJECT(
				LinkedListPop( & CritInterruptHandlerList ),
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

	CritInterruptDecrement();
}

void CritInterruptRegisterHandler(
		struct HANDLER_OBJECT * handler,
		HANDLER_FUNCTION foo,
		void * context )
{

	handler->Function = foo;
	handler->Context = context;

	HandlerRegister( handler );

	InterruptDisable();
	LinkedListEnqueue( &handler->Link.LinkedListLink,
			& CritInterruptHandlerList );
	InterruptEnable();

	HalRaiseCritInterrupt();
}

/*
 * Called by the SoftInterrupt unit when he determines that he
 * does not know what interrupt mask to apply.
 * If CritInterrupts are disabled we will apply that mask,
 * otherwise we will enable all interrupts.
 */
void CritInterruptDefer()
{
	if( CritInterruptLevel > 0 )
	{
		//Crit Interrupts are disabled, so we should set the 
		//crit disabled mask.
		HalDisableCritInterrupts();
	}
	else
	{
		//Crit interrupts are allowed. Because crit interrupts have
		//lowest priority we know that all interrupts should be allowed.
		HalEnableInterrupts();
	}
}

