#include"handler.h"

void HandlerInit( 
		struct HANDLER_OBJECT * handler, 
		HANDLER_FUNCTION * foo)
{
	//TODO do we need link init???
	handler->Function = foo;
}

void HandlerRun( struct HANDLER_OBJECT * handler, void * context )
{
	handler->Function(context);
}
