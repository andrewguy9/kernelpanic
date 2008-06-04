#include"context.h"
#include"mutex.h"

/*
 * This lock protects the current
 * Stack. This allows for
 * people to set the next stack
 * and switch into it atomically.
 */
struct MUTEX ContextMutex;

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

BOOL ContextIsLocked( )
{
	return MutexIsLocked( &ContextMutex );
}

void ContextStartup( )
{
	MutexInit( &ContextMutex );
}
