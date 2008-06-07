#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utils/utils.h"
#include"hal.h"
#include"thread.h"

void ContextInit( struct STACK * Stack, char * pointer, COUNT Size, THREAD_MAIN Foo );

BOOL ContextLock( );

void ContextUnlock( );

BOOL ContextIsCritical( );

void ContextStartup( );

struct THREAD * ContextGetActiveThread();

void ContextSetNextThread( struct THREAD * thread );

void
__attribute__((naked,__INTR_ATTRS))
ContextSwitch();

#endif
