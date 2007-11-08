#include"interrupt.h"
#include"hal.h"

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

	InterruptRunPostHandlers();

	InterruptLevel--;

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
	InterruptLevel--;
	if( InterruptLevel == 0 )
	{
		InterruptRunPostHandlers();
		HalEnableInterrupts();
	}
}

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
