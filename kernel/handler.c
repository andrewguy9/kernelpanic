#include"handler.h"

void HandlerInit( struct HANDLER_OBJECT * handler )
{
	handler->State = HANDLER_READY;
}

void HandlerRegister( struct HANDLER_OBJECT * handler )
{
	ASSERT( handler->State == HANDLER_READY || handler->State == HANDLER_RUNNING );
	handler->State = HANDLER_QUEUED;
}

void HandlerRun(struct HANDLER_OBJECT * handler )
{
	ASSERT( handler->State == HANDLER_QUEUED );
	handler->State = HANDLER_RUNNING;
}

void HandlerFinish( struct HANDLER_OBJECT * handler )
{
	ASSERT( handler->State == HANDLER_RUNNING );
	handler->State = HANDLER_READY;
}

BOOL HandlerIsFinished( struct HANDLER_OBJECT * handler )
{
	return handler->State == HANDLER_READY;
}
