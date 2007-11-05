#include"kernel/interrupt.h"
#include"hal.h"

//
//Interrupt Variables
//

COUNT InterruptLevel;
BOOL InPostInterruptHandler;
struct LINKED_LIST PostInteruptHandlerList;

//
//Internal Helper Routines.
//

void InterruptRunPostHandlers()
{
	//TODO
}

//
//Unit Management
//

void InterruptStartup()
{
	InterruptLevel = 1;//Will be reset to 0 when startup completes
	InPostInterruptHanlder = FALSE;
	LinkedListInit( & PostInteruptHanlderList );
}


//
//Handle Interrupt Entry and Exit
//

void InterruptStart()
{
	ASSERT( HalIsAtomic() && InterruptLevel == 0,
			"Interrupt level is inconsistent with start of an ISR",
			INTERRUPT_START_INTERRUPTS_INCONSISTENT);

	InterruptLevel++;
	
}

void InterruptEnd()
{
	ASSERT( HalIsAtomic() && InterruptLevel == 1,
			"Interrupt level is inconsistent with end of an ISR",
			INTERRUPT_END_INTERRUPTS_INCONSISTENT);

	InterruptLevel--;

	InterruptRunPostHandlers();

}

void InterruptRegisterPostHandler( 
		struct HANDLER_OBJECT * object,
		HANDLER_FUNCTION handler,
		void *arg)
{
	ASSERT( HalIsAtomic(), 
			"Access to the Post handler list must be atomic.",
			INTERRUPT_POST_HANDLER_REGISTER_NOT_ATOMIC);
	ASSERT( ! object->Active.
			"Adding already active post handler",
			INTERRUPT_POST_HANDLER_REGISTER_ALREADY_ACTIVE);

	object->Handler = Handler;
	object->Argument = arg;
	object->Active = TRUE;
	LinkedListInsert( (struct LINKED_LIST_LINK *) object, 
			PostInteruptHandlerList );
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
	struct POST_INTERRUPT_LIST_ITEM * listItem;

	InterruptLevel--;
	if( InterruptLevel == 0 )
	{
		InterruptRunPostHandlers();
		HalEnableInterrupts();
	}
}

void InterruptIsAtomic()
{
	//
	//If HalIsAtomic is true, 
	//then the InterruptLevel should be positive,
	//since we are physically atomic.
	//If HalIsAtomic is false,
	//then interrupt level should be 0, because we have interrupts
	//enabled.
	//
	ASSERT( HalIsAtomic() ? 
				InterruptLevel > 0 :
			   	InterruptLevel == 0,
			"InterruptIsAtomic wrong interrupt mode",
			INTERRUPT_IS_ATOMIC_WRONG_STATE);

	return HalIsAtomic;
}
