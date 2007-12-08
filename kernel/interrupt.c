#include"interrupt.h"
#include"hal.h"
#include"mutex.h"

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
 * To prevent infinite interrupt nesting we use the PostInterruptHandlerLock
 * to only allow the bottom interrupt on the stack to process post handlers.
 */

//
//Interrupt Variables
//

COUNT InterruptLevel;
struct MUTEX PostInterruptHandlerLock;
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
	if( MutexLock( & PostInterruptHandlerLock ) )
	{
		//Prevent recursion. Only the bottom level 
		//interrupt sould run these handlers.
		return;
	}
	else
	{
		while( ! LinkedListIsEmpty( & PostInterruptHandlerList ) )
		{
			//fetch handler object
			handler = BASE_OBJECT( 
					LinkedListPop(&PostInterruptHandlerList), 
					struct HANDLER_OBJECT, 
					Link);
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
		MutexUnlock( & PostInterruptHandlerLock );
	}
}

//
//Unit Management
//

//Run at kernel startup to iniialize flags.
void InterruptStartup()
{
	InterruptLevel = 1;//Will be reset to 0 when startup completes
	MutexInit( & PostInterruptHandlerLock );
	LinkedListInit( & PostInterruptHandlerList );
}


//
//Handle Interrupt Entry and Exit
//

/*
 * Should be called as the first action in EVERY interrupt. 
 *
 * Hardware sets the interrupt flag implicitly
 * when an ISR is triggered. This does not update the interrupt level.
 *
 * We have to call this function to update the system's state
 * to make the interrupt level reflect the machine state.
 *
 * Failure to call this method will make other systems think we 
 * are not atomic and will cause failures.
 */
void InterruptStart()
{
	ASSERT( (HalIsAtomic() && InterruptLevel == 0),
			INTERRUPT_START_INTERRUPTS_INCONSISTENT,
			"Interrupt level is inconsistent with \
			start of an ISR");

	InterruptLevel++;

}

/*
 * Should be called as the last action in EVERY interrupt.
 *
 * Returning from an interrupt will reset the status flag to 
 * it's prior value, including the interrupt flag.
 *
 * Interrupts should call InterruptEnd as their last statement
 * to decrement the level so its consistent with the flag on 
 * interrupt return.
 */
void InterruptEnd()
{
	ASSERT( HalIsAtomic() && InterruptLevel == 1,
			INTERRUPT_END_INTERRUPTS_INCONSISTENT,
			"Interrupt level is inconsistent with end of an ISR");

	InterruptLevel--;

	InterruptRunPostHandlers();
}

/*
 * Registers a handler object which will force funtion handler to be 
 * called before control is returned to a thread from an interrupt or 
 * critical section. 
 *
 * Must be called inside of an atomic section.
 *
 * Users of InterruptRegisterPostHandler should be careful to not
 * double register a handler. For instance:
 * The system runs an interrupt which registers handlers foo and bar.
 * While we are running foo, the interrupt fires again. Foo can be 
 * re-registered because before it was run all of its values were saved
 * to the stack, but bar has not been saved and is still in the queue.
 * Re-registering bar will cause the list of objects to be corrupted.
 */
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
	LinkedListEnqueue( &object->Link.LinkedListLink, 
			& PostInterruptHandlerList );
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
	
	BOOL atomic = HalIsAtomic();

	ASSERT( atomic ? 
				InterruptLevel > 0 :
			   	InterruptLevel == 0,
			INTERRUPT_IS_ATOMIC_WRONG_STATE,
			"InterruptIsAtomic wrong interrupt mode");

	return atomic;
}

/*
 * Should be used to verify that we are in 
 * a post interrupt handler. This should 
 * only be used in assertions and is not
 * gauranteed to be accurate.
 */
BOOL InterruptIsInPostHandler()
{
	//
	//If we are in a post handler, then we should not
	//be atomic, but interrupt level should be positive.
	//
	ASSERT( MutexIsLocked( & PostInterruptHandlerLock) ? ! HalIsAtomic() && InterruptLevel == 0 : TRUE,
		INTERRUPT_IS_POST_HANDLER_WRONG_STATE,
		"InterruptIsPostHandler is in handler, but\
		interrupt flag or interrupt level invalid");

	return MutexIsLocked( & PostInterruptHandlerLock );
}
