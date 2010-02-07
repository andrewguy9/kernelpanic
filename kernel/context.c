#include"context.h"
#include"mutex.h"
#include"interrupt.h"
#include"watchdog.h"

/*
 * Context Unit:
 * The context unit manages "what context is on the stack".
 * In other words the context unit allows for critical sections
 * by preventing context switches. It is solely responsible for
 * calling the HAL's context switch code.
 *
 * The context unit manages the context switch lock in similarly
 * to how the interrupt unit manages the interrupt flag. 
 *
 * The rule is that you lock the stack by entering a critical
 * section by calling CritInterruptStart() 
 */

struct MACHINE_CONTEXT * ActiveStack;
struct MACHINE_CONTEXT * NextStack;

/*
 * Sets up a machine context for a future thread.
 */
void ContextInit( struct MACHINE_CONTEXT * MachineState, char * Pointer, COUNT Size, STACK_INIT_ROUTINE Foo, INDEX debugFlag )
{
#ifdef DEBUG
	int cur;
#endif

	//Set up the watchdog flag.
	MachineState->Flag = debugFlag;
	WatchdogAddFlag( debugFlag );

	//initialize stack
	if( Size != 0 )
	{
#ifdef DEBUG
		//Write pattern over stack so we can expose
		//variable initialization errors.
		for(cur=0; cur<Size; cur++)
			Pointer[cur] = 0xaa;
#endif
		//Populate regular stack
		InterruptDisable();
		HalCreateStackFrame( MachineState, Pointer, Foo, Size );
		InterruptEnable();
	}
	else
	{
		//Populate stack for idle thread (machine's start thread).
		HalGetInitialStackFrame( MachineState );

#ifdef DEBUG
		MachineState->TimesRun = 0;
		MachineState->TimesSwitched = 0;

		MachineState->High = (char*) -1;
		MachineState->Low = 0;
#endif
	}
}

void ContextStartup( )
{
	NextStack = NULL;
	ActiveStack = NULL;
}

void ContextSetNextContext( struct MACHINE_CONTEXT * stack )
{
	ASSERT( NextStack == NULL );

	NextStack = stack;
}

/*
 * Should only be called at startup when the thread 
 * is specified by the scheduler.
 */
void ContextSetActiveContext( struct MACHINE_CONTEXT * stack )
{
	ASSERT( InterruptIsAtomic() );
	ASSERT( ActiveStack == NULL );
	ASSERT( NextStack == NULL );

	ActiveStack = stack;

}

void ContextSwitch()
{
	ASSERT( InterruptIsAtomic() );

	//We need to update the watchdog for the next thread.
	if( NextStack != NULL )
	{
		WatchdogNotify( NextStack->Flag );
	}


	//We are in critical section,
	//lets see if we have a thread picked to run.
	if( NextStack == NULL )
	{
		//we are critical but no thread was picked, so we dont 
		//have to do a context switch.
	}
	else if( NextStack != ActiveStack )
	{
		//The NextStack is set, so we need to context switch.
#ifdef DEBUG
		NextStack->TimesSwitched++;
		NextStack->TimesRun++;
#endif
		//now that the system looks like the switch has
		//happened, go ahead and do the switch.
		HalContextSwitch( );
	}
	else
	{
		//we are critical but the thread was the same,
		//so dont bother doing context switch. 
#ifdef DEBUG
		NextStack->TimesRun++;
#endif
		NextStack = NULL;
	}
	
	//We should be atomic, non-critical with no next stack.
	ASSERT( InterruptIsAtomic() );
	ASSERT( NextStack == NULL );
}

struct MACHINE_CONTEXT * ContextGetContext( )
{
	return ActiveStack;
}
