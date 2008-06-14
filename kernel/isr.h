#ifndef ISR_H
#define ISR_H

#include"handler.h"
#include"../utils/linkedlist.h"

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

void IsrStartup();

//
//Handle Interrupt Entry and Exit
//

typedef void (ISR_HANDLER) (void);

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
