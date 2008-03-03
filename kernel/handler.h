#ifndef HANDLER_H
#define HANDLER_H

#include"../utils/link.h"

/*
 * Handlers are a generic tool for managing callbacks. 
 * They use a generic link so they can be used in any kernel
 * data structure. The handler unit provides means of
 * initializing and executing handlers. 
 * 
 * The HANLDER_FUNCTION pointer requires a void * argument,
 * which should be the context for your specific application of 
 * handlers. 
 *
 * You should pass the containing structure so that you can have 
 * the handler re-issue itself if it encounters an issue.
 */

typedef void (HANDLER_FUNCTION)( void * Argument );

struct HANDLER_OBJECT
{
	union LINK Link;
	HANDLER_FUNCTION * Function;
};


BOOL HandlerIsStored( struct HANDLER_OBJECT * handler );

void HandlerInit( 
		struct HANDLER_OBJECT * handler, 
		HANDLER_FUNCTION * foo);

void HandlerRun( struct HANDLER_OBJECT * handler, void * context );

#endif
