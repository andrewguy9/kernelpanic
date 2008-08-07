#include"context.h"
#include"mutex.h"
#include"interrupt.h"

/*
 * This lock protects the current
 * Stack. This allows for
 * people to set the next stack
 * and switch into it atomically.
 */
struct MUTEX ContextMutex;

struct MACHINE_CONTEXT * ActiveStack;
struct MACHINE_CONTEXT * NextStack;

void ContextInit( struct MACHINE_CONTEXT * MachineState, char * Pointer, COUNT Size, STACK_INIT_ROUTINE Foo )
{
	//initialize stack
	if( Size != 0 )
	{//Populate regular stack
		HalCreateStackFrame( MachineState, Pointer, Foo, Size );
	}
	else
	{//Populate stack for idle thread (machine's start thread).
		HalGetInitialStackFrame( MachineState );
#ifdef DEBUG
		MachineState->High = (char*) -1;
		MachineState->Low = 0;
#endif
	}
}

/*
 * Lock the current stack frame so that 
 * no you can mess with the stack.
 */
BOOL ContextLock( )
{
	return MutexLock( &ContextMutex );
}

/*
 * Unlock the stack frame.
 * Does no switching.
 */
void ContextUnlock( )
{
	MutexUnlock( &ContextMutex );
}

BOOL ContextIsCritical( )
{
	return MutexIsLocked( &ContextMutex );
}

void ContextStartup( struct MACHINE_CONTEXT * startContext )
{
	MutexInit( &ContextMutex );
	NextStack = NULL;
	ActiveStack = startContext;
}

void ContextSetNextContext( struct MACHINE_CONTEXT * stack )
{
	ASSERT( ContextIsCritical() );
	ASSERT( NextStack == NULL );

	NextStack = stack;
}

void ContextSwitch()
{
	struct MACHINE_CONTEXT * oldStack;

	ASSERT( InterruptIsAtomic() );
	ASSERT( HalIsAtomic() );

	if( MutexIsLocked( &ContextMutex ) )
	{
		//We are in critical section,
		//lets see if we have a thread picked to run.
		if( NextStack == NULL )
		{
			//we are critical but no thread was picked, so we dont 
			//have to do a context switch.
			InterruptIncrement();
			ASSERT(HalIsAtomic() && !InterruptIsAtomic());
			//returning will restore the machine state.
			return;
		}
		else
		{
			//we have to do a switch.
			//get system state into post switch status.
			oldStack = ActiveStack;
			ActiveStack = NextStack;
			NextStack = NULL;

			MutexUnlock( &ContextMutex );
			InterruptDecrement();

			//now that the system looks like the switch has
			//happened, go ahead and do the switch.

			HalContextSwitch(oldStack,ActiveStack);
			return;
		}
	}
	//We are not in a critical section, so no new thread could have been
	//picked, no context switch needed.
	InterruptDecrement();
	ASSERT(HalIsAtomic() && !InterruptIsAtomic());
	//returning will restore the machine state.
	return;
}

struct MACHINE_CONTEXT * ContextGetContext( )
{
	return ActiveStack;
}
