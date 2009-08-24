#ifndef ISR_H
#define ISR_H

#include"handler.h"
#include"../utils/linkedlist.h"

void IsrStartup();

//
//Handle Interrupt Entry and Exit
//

void IsrStart();

void IsrEnd();

//
//Functions for post handlers
//

void IsrRegisterPostHandler( 
		struct POST_HANDLER_OBJECT * postObject,
		HANDLER_FUNCTION foo,
		void * context);

#endif /* ISR_H */
