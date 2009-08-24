#include"handler.h"

void HandlerInit( struct POST_HANDLER_OBJECT * handler )
{
	handler->State = HANDLER_READY;
}

void HandlerRegister( struct POST_HANDLER_OBJECT * handler )
{
	ASSERT( handler->State == HANDLER_READY || handler->State == HANDLER_RUNNING );
	handler->State = HANDLER_QUEUED;
}

void HandlerRun(struct POST_HANDLER_OBJECT * handler )
{
	ASSERT( handler->State == HANDLER_QUEUED );
	handler->State = HANDLER_RUNNING;
}

void HandlerFinish( struct POST_HANDLER_OBJECT * handler )
{
	ASSERT( handler->State == HANDLER_RUNNING );
	handler->State = HANDLER_READY;
}
