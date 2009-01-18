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
 * The rule is that locking the ContextMutex with ContextLock() 
 * "locks the stack".
 */

/*
 * This lock protects the current
 * Stack. This allows for
 * people to set the next stack
 * and switch into it atomically.
 */
struct MUTEX ContextMutex;

struct MACHINE_CONTEXT * ActiveStack;
struct MACHINE_CONTEXT * NextStack;

#ifdef DEBUG
volatile COUNT ContextNumSwitches;
#endif

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

/*
 * Call this to determine if a context switch has already been scheduled.
 */
BOOL ContextCanSwitch()
{
	ASSERT( ContextIsCritical() );

	if( NextStack == NULL )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void ContextStartup( )
{
	MutexInit( &ContextMutex, TRUE );
	NextStack = NULL;
	ActiveStack = NULL;

#ifdef DEBUG
	ContextNumSwitches = 0;
#endif
}

void ContextSetNextContext( struct MACHINE_CONTEXT * stack )
{
	ASSERT( ContextIsCritical() );
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
	ASSERT( MutexIsLocked( &ContextMutex ) );

	ASSERT( ++ContextNumSwitches == 1 );
	
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
		MutexUnlock( &ContextMutex );
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
		MutexUnlock( &ContextMutex );

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
		MutexUnlock( &ContextMutex );
	}
	
	ASSERT( ContextNumSwitches-- == 1 );

	//We should be atomic, non-critical with no next stack.
	ASSERT( InterruptIsAtomic() );
	ASSERT( !MutexIsLocked( &ContextMutex ) );
	ASSERT( NextStack == NULL );
}

struct MACHINE_CONTEXT * ContextGetContext( )
{
	return ActiveStack;
}
