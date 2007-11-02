#include"kernel/interrupt.h"
#include"../utils/utils.h"
#include"hal.h"

//
//Interrupt Variables
//

COUNT InterruptLevel;
BOOL InPostInterruptHandler;
struct LINKED_LIST PostInteruptHandlerList;

//
//Internal Interrupt Calls.
//

void InterruptRunPostHandlers()
{
	ASSERT( InterruptLevel == 0 && HalIsAtomic(),
			"InterruptRunPostHandlers has wrong interrupt settings",
			INTERRUPT_RUN_POST_HANDLERS_INCONSISTENT_STATE);

	if( InPostInterruptHandler )
	{
		//We are already processing these handlers, 
		//so we should just go ahead and enable interrupts.
		HalEnableInterrupts();
	}
	else
	{
		InPostInterruptHandler = TRUE;
		while( ! LinkListIsEmpty( & PostInterruptHandlerList ) )
		{
			listItem = ( struct POST_INTERRUPT_LIST_ITEM * )
				LinkedListPop( & PostInterruptHandlerList );
			HalEnableInterrupts();
			listItem->Handler( listItem->Argument );
			HalDisableInterrupts();
		}
	}
}

void InterruptStart()
{
	InterruptLevel++;
	ASSERT( HalIsAtomic() && InterruptLevel == 1,
			"Interrupt level is consistent",
			INTERRUPT_START_INTERRUPTS_INCONSISTENT);
}

void InterruptEnd()
{
	ASSERT( HalIsAtomic() && InterruptLevel == 1,
			"Interrupt level is consistent",
			INTERRUPT_END_INTERRUPTS_INCONSISTENT);
	InterruptLevel--;

	InterruptRunPostHandlers();
}

void GeneralInterruptHandler( INTERRUPT_HANDLER * handler )
{
	InterruptStart();
	//TODO
	InterruptEnd();
}

//
//Public functions
//

void InterruptStartup()
{
	InterruptLevel = 1;
	InPostInterruptHanlder = FALSE;
	LinkedListInit( & PostInteruptHanlderList );
}

void InterruptInit( 
		INTERRUPT_HANDLER * handler, 
		void * signal )
{
	//TODO
}

void InterruptPostHandlerRegister( 
		POST_INTERRUPT_HANDLER * Handler,
		void *arg,
		struct POST_INTERRUPT_LIST_ITEM * listItem )
{
	ASSERT( HalIsAtomic(), 
			"Access to the Post handler list must be atomic.",
			INTERRUPT_POST_HANDLER_REGISTER_NOT_ATOMIC);

	listItem->Handler = Handler;
	listItem->Argument = arg;
	LinkedListInsert( (struct LINKED_LIST_LINK *) listItem, 
			PostInteruptHandlerList );
}

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
	ASSERT( HalIsAtomic() ? 
				InterruptLevel > 0 :
			   	InterruptLevel == 0,
			"InterruptIsAtomic wrong interrupt mode",
			INTERRUPT_IS_ATOMIC_WRONG_STATE);

	return HalIsAtomic;
}
