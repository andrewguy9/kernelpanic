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

struct STACK * ActiveStack;
struct STACK * NextStack;

void ContextInit( struct STACK * Stack, char * pointer, COUNT Size, STACK_INIT_ROUTINE Foo )
{
	//initialize stack
	if( Size != 0 )
	{//Populate regular stack
		Stack->Pointer = HalCreateStackFrame( pointer, Foo, Size );
		//Save the stack size.
#ifdef DEBUG
		Stack->High = pointer + Size;
		Stack->Low = pointer;
#endif
	}
	else
	{//Populate stack for idle thread
		Stack->Pointer = NULL;
#ifdef DEBUG
		Stack->High = (char*) -1;
		Stack->Low = 0;
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

void ContextStartup( struct STACK * startStack )
{
	MutexInit( &ContextMutex );
	NextStack = NULL;
	ActiveStack = startStack;
}

void
HAL_NAKED_FUNCTION
ContextSwitch()
{
	//perfrom context switch
	HAL_SAVE_STATE
	
	HAL_SAVE_SP( ActiveStack->Pointer );

	//We need to have the Isr unit to think were back in the thread
	//context, and the machine to be atomic.
	ASSERT( HalIsAtomic() && !InterruptIsAtomic() );

	ASSERT( MutexIsLocked( &ContextMutex ) );

	//Check to see if stack has overflowed.
#ifdef DEBUG
	ASSERT( ASSENDING( 
				(unsigned int) ActiveStack->Low, 
				(unsigned int) ActiveStack->Pointer, 
				(unsigned int) ActiveStack->High ) );
#endif
	//Switch threads
	ActiveStack = NextStack;
	NextStack = NULL;

	HAL_SET_SP( ActiveStack->Pointer );

	HAL_RESTORE_STATE
}

void ContextSetNextContext( struct STACK * stack )
{
	ASSERT( ContextIsCritical() );
	ASSERT( NextStack == NULL );

	NextStack = stack;
}

BOOL ContextSwitchNeeded()
{
	//We need to have the machine in a physically atomic state
	//and we need the isr unit to think that we already returned from
	//interrupt.
	ASSERT( HalIsAtomic() && ! InterruptIsAtomic() );

	if( MutexIsLocked( &ContextMutex ) )
	{
		//We are in critical section,
		//lets see if we have a thread picked to run.
		if( NextStack != NULL )
		{
			//We need a context switch.
			return TRUE;
		}
		else
		{
			//we are critical but no thread was picked.
			return FALSE;
		}
	}
	//We are not in a critical section, so no new thread could have been
	//picked, no context switch needed.
	return FALSE;
}

struct STACK * ContextGetStack( )
{
	return ActiveStack;
}
