#include"interrupt.h"
#include"hal.h"
#include"context.h"
#include"mutex.h"

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
//TODO MOVE TO ISR
struct LINKED_LIST PostInterruptHandlerList;//List of PostInterruptHandlers
//TODO MOVE TO ISR
struct MUTEX PostHandlerMutex;

//
//Internal Helper Routines.
//
//MOVE TO ISR
void InterruptRunPostHandlers()
{
	struct HANDLER_OBJECT * handler;
	struct POST_HANDLER_OBJECT * postHandler;
	HANDLER_FUNCTION * func;

	ASSERT( HalIsAtomic(),0,"");

	ASSERT( MutexIsLocked(&PostHandlerMutex), 0, "" );

	while( ! LinkedListIsEmpty( & PostInterruptHandlerList ) )
	{
		//Pull data out of structure 
		handler = BASE_OBJECT( 
				LinkedListPop(&PostInterruptHandlerList), 
				struct HANDLER_OBJECT, 
				Link);
		postHandler = BASE_OBJECT(
				handler,
				struct POST_HANDLER_OBJECT,
				HandlerObj);
		func = postHandler->HandlerObj.Function;

		//mark stucture as unqueued
		ASSERT(postHandler->Queued,
				INTERRUPT_RUN_POST_HANDLERS_NOT_QUEUED,
				"post interrupt handler not queued");

		postHandler->Queued = FALSE;

		//Change State to "not atomic"
		InterruptLevel--;
		HalEnableInterrupts();

		//Run the handler
		//We pass a pointer to the handler itself so 
		//the handler can reschedule itself.
		func(postHandler);

		//make atomic again.
		HalDisableInterrupts();
		InterruptLevel++;
	}
}

//
//Unit Management
//

//Run at kernel startup to initialize flags.
void InterruptStartup()
{
	ASSERT( HalIsAtomic(),
			INTERRUPT_STARTUP_NOT_ATOMIC,
			"we started in inconsistant state" );

	InterruptLevel = 1;//Will be reset to 0 when startup completes
	MutexInit(&PostHandlerMutex);//TODO MOVE TO ISR
	LinkedListInit( & PostInterruptHandlerList );//TODO MOVE TO ISR
}


//
//Handle Interrupt Entry and Exit
//

//TODO MOVE TO ISR
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
 *
 * We also have to tell the system that another interrupt is on the
 * stack, so we dont try to fire post handlers.
 */
void InterruptStart()
{
	ASSERT( (HalIsAtomic() && InterruptLevel == 0),
			INTERRUPT_START_INTERRUPTS_INCONSISTENT,
			"Interrupt level is inconsistent with \
			start of an ISR");

	InterruptLevel++;
}

//TODO MOVE TO ISR
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

	if( MutexLock(&PostHandlerMutex) )
	{
		InterruptRunPostHandlers();

		MutexUnlock(&PostHandlerMutex);

		ContextSwitchIfNeeded();
	}

	InterruptLevel--;
}

//TODO MOVE TO ISR
/*
 * Registers a handler object which will force funtion handler to be 
 * called before control is returned to a thread from an interrupt.
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
		struct POST_HANDLER_OBJECT * postObject,
		HANDLER_FUNCTION foo,
		void * context)
{
	ASSERT( HalIsAtomic(),
			INTERRUPT_POST_HANDLER_REGISTER_NOT_ATOMIC,
			"Access to the Post handler list must be atomic.");
	ASSERT( ! postObject->Queued,
			INTERRUPT_POST_HANDLER_REGISTER_ALREADY_ACTIVE,
			"Adding already active post handler");

	postObject->HandlerObj.Function = foo;
	postObject->Context = context;

	//mark handler as queued so we dont try to mess with it.
	postObject->Queued = TRUE;

	//Queue handler to be run
	LinkedListEnqueue( &postObject->HandlerObj.Link.LinkedListLink, 
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
	ASSERT( HalIsAtomic(), 0, "" );
	ASSERT( InterruptLevel > 0,
			INTERRUPT_ENABLE_OVER_ENABLED,
			"We cannot enable interrupts when \
			InterruptLevel is not positive");
	InterruptLevel--;
	if( InterruptLevel == 0 )
	{
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
