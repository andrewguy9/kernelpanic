#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"../utils/linkedlist.h"
#include"../utils/utils.h"

//
//Unit Management
//

void InterruptStartup();

//
//Handle Interrupt Entry and Exit
//

typedef void (INTERRUPT_HANDLER) (void);

void Interruptstart();

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

BOOL InterruptIsAtomic();

#endif
