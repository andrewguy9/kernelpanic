#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/linkedlist.h"

typedef void (INTERRUPT_HANDLER) (void);

typedef void (POST_INTERRUPT_HANDLER)(void *arg);

struct POST_INTERRUPT_LIST_ITEM
{
	struct LINKED_LIST_LINK * Link;
	INTERRUPT_HANDLER * Handler;
	void *Argument;
};

void InterruptStartup();

void InterruptInit( INTERRUPT_HANDLER * handler,
		void * signal );

void InterruptPostHandlerRegister( 
		POST_INTERRUPT_HANDLER * Handler,
		struct POST_INTERRUPT_LIST_ITEM * listItem );

void InterruptDisable();

void InterruptEnable();

void InterruptIsAtomic();

#endif
