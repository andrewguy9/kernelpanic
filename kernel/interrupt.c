#include"interrupt.h"
#include"hal.h"

/*
 * Interrupt Unit Description
 * The interrupt unit provides additional control to the interrupt flag.
 *
 * InterruptLevel is used to track the desired state of the interrupt flag.
 * When its greater than 0, then interrupts should be diabled. 
 *
 * When exiting an atomic section, we reenable interrupts and run 
 * the post interrupt handlers with the interrupts off and then return.
 * This allows the heavy lifting in interrupts to be done with interrupts
 * enabled. The benefit is really short atomic sections, by running heavy
 * operations in post interrupt handlers.
 *
 * To prevent infinite interrupt nesting we use the InPostInterruptHandler 
 * to only allow the bottom interrupt on the stack to process post handlers.
 */

//
//Interrupt Variables
//

COUNT InterruptLevel;
BOOL InPostInterruptHandler;
struct LINKED_LIST PostInterruptHandlerList;

//
//Internal Helper Routines.
//

void InterruptRunPostHandlers()
{
	struct HANDLER_OBJECT * handler;
	void * argument;
	HANDLER_FUNCTION * foo;

	//Check to make sure we are bottom handler.
	if( InPostInterruptHandler )
	{
		//Prevent recursion. Only the bottom level 
		//interrupt sould run these handlers.
		return;
	}

	InPostInterruptHandler = TRUE;
	while( ! LinkedListIsEmpty( & PostInterruptHandlerList ) )
	{
		//fetch handler object
		handler = (struct HANDLER_OBJECT*) LinkedListPop(
				&PostInterruptHandlerList );
		//fech values from object so we can reuse it.
		argument = handler->Argument;
		foo = handler->Handler;
		//mark stucture so handler can reschedule itself.
		handler->Enabled = FALSE;
		//run handler.
		HalEnableInterrupts();
		foo( argument );
		HalDisableInterrupts();
	}
	InPostInterruptHandler = FALSE;
}

//
//Unit Management
//

void InterruptStartup()
{
	InterruptLevel = 1;//Will be reset to 0 when startup completes
	InPostInterruptHandler = FALSE;
	LinkedListInit( & PostInterruptHandlerList );
}


//
//Handle Interrupt Entry and Exit
//

void InterruptStart()
{
	ASSERT( (HalIsAtomic() && InterruptLevel == 0),
			INTERRUPT_START_INTERRUPTS_INCONSISTENT,
			"Interrupt level is inconsistent with \
			start of an ISR");

	InterruptLevel++;
	
}

void InterruptEnd()
{
	ASSERT( HalIsAtomic() && InterruptLevel == 1,
			INTERRUPT_END_INTERRUPTS_INCONSISTENT,
			"Interrupt level is inconsistent with end of an ISR");

	InterruptLevel--;

	InterruptRunPostHandlers();
}

void InterruptRegisterPostHandler( 
		struct HANDLER_OBJECT * object,
		HANDLER_FUNCTION handler,
		void *arg)
{
	ASSERT( HalIsAtomic(),
			INTERRUPT_POST_HANDLER_REGISTER_NOT_ATOMIC,
			"Access to the Post handler list must be atomic.");
	ASSERT( ! object->Enabled,
			INTERRUPT_POST_HANDLER_REGISTER_ALREADY_ACTIVE,
			"Adding already active post handler");

	object->Handler = handler;
	object->Argument = arg;
	object->Enabled = TRUE;
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) object, 
			& PostInterruptHandlerList );
}

//
//Handle Atomic Sections
//

void InterruptDisable()
{
	HalDisableInterrupts();
	InterruptLevel++;
}

void InterruptEnable()
{
	ASSERT( InterruptLevel > 0,
			INTERRUPT_ENABLE_OVER_ENABLED,
			"We cannot enable interrupts when \
		   	InterruptLevel is not positive");
	InterruptLevel--;
	if( InterruptLevel == 0 )
	{
		InterruptRunPostHandlers();
		HalEnableInterrupts();
	}
}

//
//Functions for Sanity Checking
//

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
	
	BOOL atomic = HalIsAtomic();

	ASSERT( atomic ? 
				InterruptLevel > 0 :
			   	InterruptLevel == 0,
			INTERRUPT_IS_ATOMIC_WRONG_STATE,
			"InterruptIsAtomic wrong interrupt mode");

	return atomic;
}

BOOL InterruptIsInPostHandler()
{
	BOOL atomic = InterruptIsAtomic();

	//
	//If we are in a post handler, then we should not
	//be atomic, but interrupt level should be positive.
	//
	ASSERT( InPostInterruptHandler ? ! HalIsAtomic() && InterruptLevel == 0 : TRUE,
		INTERRUPT_IS_POST_HANDLER_WRONG_STATE,
		"InterruptIsPostHandler is in handler, but\
		interrupt flag or interrupt level invalid");

	return InPostInterruptHandler;
}
