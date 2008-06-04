#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utils/utils.h"
#include"hal.h"

struct STACK
{
	char * Pointer;
	char * High;
	char * Low;
};

void ContextInit( struct STACK * Stack, char * pointer, COUNT Size, THREAD_MAIN Foo );

BOOL ContextLock( );

void ContextUnlock( );

BOOL ContextIsLocked( );

void ContextStartup( );
#endif
