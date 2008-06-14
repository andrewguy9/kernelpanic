#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/linkedlist.h"
#include"../utils/utils.h"

#include"handler.h"

//
//Structure for Post Handlers
//
//TODO MOVE TO ISR
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
//TODO MOVE TO ISR
void InterruptStartup();

//
//Handle Interrupt Entry and Exit
//
//TODO MOVE TO ISR
typedef void (INTERRUPT_HANDLER) (void);
//TODO MOVE TO ISR
void InterruptStart();
//TODO MOVE TO ISR
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

//
//Functions for post handlers
//
//MOVE TO ISR
void InterruptRegisterPostHandler( 
		struct POST_HANDLER_OBJECT * postObject,
		HANDLER_FUNCTION foo,
		void * context);
#endif
