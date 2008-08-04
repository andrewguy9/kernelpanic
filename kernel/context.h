#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utils/utils.h"
#include"hal.h"
#include"stack.h"

void ContextInit( struct STACK * Stack, char * pointer, COUNT Size, STACK_INIT_ROUTINE Foo );

BOOL ContextLock( );

void ContextUnlock( );

BOOL ContextIsCritical( );

void ContextStartup( struct STACK * stack );

void ContextSetNextContext( struct STACK * stack );

BOOL ContextSwitchNeeded();

void
HAL_NAKED_FUNCTION
ContextSwitch();


struct STACK * ContextGetStack( );
#endif
