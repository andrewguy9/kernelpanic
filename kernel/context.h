#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utils/utils.h"
#include"hal.h"
#include"thread.h"
#include"stack.h"

void ContextInit( struct STACK * Stack, char * pointer, COUNT Size, STACK_INIT_ROUTINE Foo );

BOOL ContextLock( );

void ContextUnlock( );

BOOL ContextIsCritical( );

void ContextStartup( );

struct THREAD * ContextGetActiveThread();

void ContextSetNextThread( struct THREAD * thread );

void ContextSwitchIfNeeded();

#endif
