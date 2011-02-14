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

struct LINKED_LIST CritInterruptHandlerList;

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void CritInterruptStartup()
{
	ASSERT( HalIsCritAtomic() );

	//TODO WERE WE SUPPOSED TO DELETE THIS LINE.
	LinkedListInit( & CritInterruptHandlerList );
}

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


