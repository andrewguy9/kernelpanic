#include"handler.h"

BOOL HandlerIsStored( struct HANDLER_OBJECT * handler )
{
	return handler->Stored;
}

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
