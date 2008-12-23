#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utils/utils.h"
#include"hal.h"

void ContextInit( struct MACHINE_CONTEXT * context, char * pointer, COUNT Size, STACK_INIT_ROUTINE Foo, INDEX debugFlag );

BOOL ContextLock( );

void ContextUnlock( );

BOOL ContextIsCritical( );

BOOL ContextCanSwitch( );

void ContextStartup( );

void ContextSetActiveContext( struct MACHINE_CONTEXT * stack );

void ContextSetNextContext( struct MACHINE_CONTEXT * stack );

void ContextSwitch();

struct MACHINE_CONTEXT * ContextGetContext( );
#endif
