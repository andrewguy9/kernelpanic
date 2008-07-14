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
		Stack->High = pointer + Size;
		Stack->Low = pointer;
	}
	else
	{//Populate stack for idle thread
		Stack->Pointer = NULL;
		Stack->High = (char*) -1;
		Stack->Low = 0;
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

	ASSERT( InterruptIsAtomic() );

	ASSERT( MutexIsLocked( &ContextMutex ) );

	//Check to see if stack has overflowed.
	ASSERT( ASSENDING( 
				(unsigned int) ActiveStack->Low, 
				(unsigned int) ActiveStack->Pointer, 
				(unsigned int) ActiveStack->High ) );

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

void ContextSwitchIfNeeded()
{
	ASSERT( InterruptIsAtomic() );

	if( MutexIsLocked( &ContextMutex ) )
	{
		//We are in critical section,
		//lets see if we have a thread picked to run.
		if( NextStack != NULL )
		{
			//Switch threads and end critical section.
			ContextSwitch();
			MutexUnlock( &ContextMutex );
		}
	}
}
