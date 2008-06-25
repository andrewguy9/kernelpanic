#include"isr.h"
#include"context.h"
#include"mutex.h"
#include"interrupt.h"

struct LINKED_LIST PostInterruptHandlerList;//List of PostInterruptHandlers
struct MUTEX PostHandlerMutex;

//
//Internal Helper Routines.
//
//MOVE TO ISR
void IsrRunPostHandlers()
{
	struct HANDLER_OBJECT * handler;
	struct POST_HANDLER_OBJECT * postHandler;
	HANDLER_FUNCTION * func;

	ASSERT( HalIsAtomic() );

	ASSERT( MutexIsLocked(&PostHandlerMutex) );

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
		ASSERT(postHandler->Queued );

		postHandler->Queued = FALSE;

		//Change State to "not atomic"
		InterruptEnable();

		//Run the handler
		//We pass a pointer to the handler itself so 
		//the handler can reschedule itself.
		func(postHandler);

		//make atomic again.
		InterruptDisable();
	}
}

//
//Unit Management
//

void IsrStartup()
{
	MutexInit( & PostHandlerMutex );
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
 *
 * We also have to tell the system that another interrupt is on the
 * stack, so we dont try to fire post handlers.
 */
void IsrStart()
{
	InterruptIncrement();
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
void IsrEnd()
{
	if( MutexLock(&PostHandlerMutex) )
	{
		IsrRunPostHandlers();

		MutexUnlock(&PostHandlerMutex);

		ContextSwitchIfNeeded();
	}

	InterruptDecrement();
}

//
//Routines for ISR's
//

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
void IsrRegisterPostHandler( 
		struct POST_HANDLER_OBJECT * postObject,
		HANDLER_FUNCTION foo,
		void * context)
{
	//Access to the Post handler list must be atomic.
	ASSERT( HalIsAtomic() );
	//We cannot add an object that is in use.
	ASSERT( ! postObject->Queued );

	postObject->HandlerObj.Function = foo;
	postObject->Context = context;

	//mark handler as queued so we dont try to mess with it.
	postObject->Queued = TRUE;

	//Queue handler to be run
	LinkedListEnqueue( &postObject->HandlerObj.Link.LinkedListLink, 
			& PostInterruptHandlerList );
}

