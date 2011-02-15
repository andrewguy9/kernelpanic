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

struct LINKED_LIST SoftInterruptHandlerList;

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void SoftInterruptStartup()
{
	ASSERT( HalIsIrqAtomic(IRQ_LEVEL_SOFT) );

	//TODO WAS I SUPPOSED TO DELETE THIS?
	LinkedListInit( &SoftInterruptHandlerList );
}

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


