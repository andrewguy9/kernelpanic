#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utils/utils.h"
#include"hal.h"

void ContextInit( struct MACHINE_CONTEXT * context, char * pointer, COUNT Size, STACK_INIT_ROUTINE Foo, INDEX debugFlag );
void ContextSwitch(struct MACHINE_CONTEXT * oldStack, struct MACHINE_CONTEXT * newStack);

#endif
