#include"handler.h"

void HandlerInit( struct HANDLER_OBJECT * handler)
{
        handler->Context = NULL;
        handler->Function = NULL;
        handler->CompleteFunction = NULL;
        handler->State = HANDLER_READY;
}

void HandlerCompetion(
                struct HANDLER_OBJECT * handler,
                HANDLER_FUNCTION * foo)
{
        ASSERT( handler->State == HANDLER_READY );
        ASSERT( !handler->CompleteFunction );
        handler->CompleteFunction = foo;
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
        HANDLER_FUNCTION * foo = handler->CompleteFunction;
        ASSERT( handler->State == HANDLER_RUNNING );
        handler->State = HANDLER_READY;

        if (foo) {
                foo(handler);
        }
}

BOOL HandlerIsFinished( struct HANDLER_OBJECT * handler )
{
        return handler->State == HANDLER_READY;
}
