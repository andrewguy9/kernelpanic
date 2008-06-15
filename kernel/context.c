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

	ASSERT( InterruptIsAtomic() );

	ASSERT( MutexIsLocked( &ContextMutex ) );

	//Check to see if stack has overflowed.
	ASSERT( ASSENDING( 
				(unsigned int) ActiveThread->Stack.Low, 
				(unsigned int) ActiveThread->Stack.Pointer, 
				(unsigned int) ActiveThread->Stack.High ) );

	//Switch threads
	ActiveThread = NextThread;
	NextThread = NULL;

	HAL_SET_SP( ActiveThread->Stack.Pointer );

	HAL_RESTORE_STATE
}

struct THREAD * ContextGetActiveThread()
{
	ASSERT( ContextIsCritical() );
	return ActiveThread;
}

void ContextSetNextThread( struct THREAD * thread )
{
	ASSERT( ContextIsCritical() );
	ASSERT( NextThread == NULL );

	NextThread = thread;
}

void ContextSwitchIfNeeded()
{
	ASSERT( InterruptIsAtomic() );

	if( MutexIsLocked( &ContextMutex ) )
	{
		//We are in critical section,
		//lets see if we have a thread picked to run.
		if( NextThread != NULL )
		{
			//Switch threads and end critical section.
			ContextSwitch();
			MutexUnlock( &ContextMutex );
		}
	}
}
