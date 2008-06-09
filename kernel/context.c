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

struct THREAD * ActiveThread;
struct THREAD * NextThread;

void ContextInit( struct STACK * Stack, char * pointer, COUNT Size, THREAD_MAIN Foo )
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

void ContextStartup( struct THREAD * startThread )
{
	MutexInit( &ContextMutex );
	NextThread = NULL;
	ActiveThread = startThread;
}

void
__attribute__((naked,__INTR_ATTRS))
ContextSwitch()
{
	//perfrom context switch
	HAL_SAVE_STATE
	
	HAL_SAVE_SP( ActiveThread->Stack.Pointer );

	ASSERT( InterruptIsAtomic(), 
			SCHEDULER_CONTEXT_SWITCH_NOT_ATOMIC,
			"Context switch must save state atomically");

	ASSERT( NextThread != NULL, 0, "" );

	//Check to see if stack is valid.
	ASSERT( ASSENDING( 
				(unsigned int) ActiveThread->Stack.Low, 
				(unsigned int) ActiveThread->Stack.Pointer, 
				(unsigned int) ActiveThread->Stack.High ),
			SCHEDULER_CONTEXT_SWITCH_STACK_OVERFLOW,
			"stack overflow");


	HAL_SET_SP( ActiveThread->Stack.Pointer );

	HAL_RESTORE_STATE
}

struct THREAD * ContextGetActiveThread()
{
	ASSERT( ContextIsCritical(), 0,"" );
	return ActiveThread;
}

void ContextSetNextThread( struct THREAD * thread )
{
	ASSERT( ContextIsCritical(), 0, "" );
	ASSERT( NextThread == NULL, 0, "" );

	NextThread = thread;
}

void ContextSwitchIfNeeded()
{
	ASSERT( InterruptIsAtomic(), 0, "" );
	ASSERT( ContextIsCritical(), 0, "" );

	//We end the critical section in the switch.
	MutexUnlock( &ContextMutex );
	if( NextThread != NULL )
	{
		ContextSwitch();
	}
}
