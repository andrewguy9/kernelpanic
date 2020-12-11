#ifndef HANDLER_H
#define HANDLER_H

#include"utils/link.h"
#include"utils/utils.h"
/*
 * Handlers are a generic tool for managing callbacks.
 * They use a generic link so they can be used in any kernel
 * data structure. The handler unit provides a state machine
 * to describe the life cycle of a callback handler.
 *
 * HANDLER_FUNCTIONs recieve a pointer to the callback structure
 * itself. This allows for handlers to re-register themselves.
 *
 * There are two examples of situations when this is desirable:
 *
 * 1) A timer which wants to be fired on a periodic basis can
 * reregister itself.
 *
 * 2) Timers are executed as PostInterruptHandlers, so they
 * get dequeued out of the timer heap and placed into the
 * Interrupt queue directly.
 */

/*
 * Init
 * |
 * HANDLER_READY<-------------
 * |                         |
 * Register<---------------  |
 * |                      |  |
 * HANDLER_QUEUED         |  |
 * |                      |  |
 * Run                    |  |
 * |                      |  |
 * HANDLER_RUNNING        |  |
 * |           |          |  |
 * Finished    ------------  |
 * |    *                    |
 * |    *                    |
 * |    CompletionHandler    |
 * |                         |
 * |                         |
 * ---------------------------
 *
 *  HANDLER_READY is the initial state of a handler object.
 *  HANDLER_QUEUED is the state of a handler which is queued and waiting for execution.
 *  HANDLER_RUNNING is the state of a handler which has been dequeued and is running.
 *
 *  When a handler is running and re-enqueues into a handler list he can
 *  transition from HANDLER_RUNNING directly to HANDLER_QUEUED.
 *
 *  When a handler finishes running and does not reschedule itself it will
 *  transition back to HANDLER_READY.
 */

enum HANDLER_STATE { HANDLER_READY=1, HANDLER_QUEUED, HANDLER_RUNNING };

struct HANDLER_OBJECT;

//Handler Functions return true if they have completed and want the owning component
//to mark them as completed. If false is retured then ownership has transitioned to a
//new component and the structure should no longer be touched.
typedef _Bool (HANDLER_FUNCTION)( struct HANDLER_OBJECT * HandlerObj );
//Handler Complete Functions are executed syncronously when a handler moves into the
//finished state.
typedef void (HANDLER_COMPLETE_FUNCTION)( struct HANDLER_OBJECT * HandlerObj );

struct HANDLER_OBJECT
{
        union LINK Link;
        HANDLER_FUNCTION * Function;
        void * Context;

        HANDLER_COMPLETE_FUNCTION * CompleteFunction;

        enum HANDLER_STATE State;
};

//Should be called on all HANDLER_OBJECTs before use.
void HandlerInit( struct HANDLER_OBJECT * handler);
//Should be called after HandlerInit.
void HandlerCompetion(
                struct HANDLER_OBJECT * handler,
                HANDLER_COMPLETE_FUNCTION * foo);
//Should be called by the owning component when a handler has been queued.
void HandlerRegister( struct HANDLER_OBJECT * handler );
//Should be called by the owning component when a handler has been selected to run.
void HandlerRun(struct HANDLER_OBJECT * handler );
//If the HANDLER_FUCTION has returned true then the owning component should call
//HandlerFinish to reset him to the HANDLER_READY state.
void HandlerFinish( struct HANDLER_OBJECT * handler );
//Used to determine if a handler has run to completion.
_Bool HandlerIsFinished( struct HANDLER_OBJECT * handler );
#endif
