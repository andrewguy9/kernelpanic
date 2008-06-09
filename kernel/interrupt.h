#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/linkedlist.h"
#include"../utils/utils.h"

#include"handler.h"

//
//Structure for Post Handlers
//

struct POST_HANDLER_OBJECT
{
	//HandlerObj.Link = Storage Mechanism
	//HandlerObj.Function = Handler Function
	struct HANDLER_OBJECT HandlerObj;

	//Context for execution
	void * Context;

	//Flag to see if currently Queued
	BOOL Queued;
};

//
//Unit Management
//

void InterruptStartup();

//
//Handle Interrupt Entry and Exit
//

typedef void (INTERRUPT_HANDLER) (void);

void InterruptStart();

void InterruptEnd();

//
//Handle Atomic Sections
//

void InterruptDisable();

void InterruptEnable();

//
//Functions for Sanity Checking
//

BOOL InterruptIsAtomic();

#endif
