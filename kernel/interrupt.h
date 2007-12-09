#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/linkedlist.h"
#include"../utils/utils.h"

#include"handler.h"

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
//Handle Post Interrupt Routines
//

void InterruptRegisterPostHandler( 
		struct HANDLER_OBJECT * object,
		HANDLER_FUNCTION handler,
		void *arg);

//
//Handle Atomic Sections
//

void InterruptDisable();

void InterruptEnable();

//
//Functions for Sanity Checking
//

BOOL InterruptIsAtomic();

BOOL InterruptIsInPostHandler();

#endif
