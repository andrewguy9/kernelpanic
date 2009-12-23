#ifndef HANDLER_H
#define HANDLER_H

#include"../utils/link.h"
#include"../utils/utils.h"
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

/*
 * Init  
 * |
 * HANDLER_READY<-----------
 * |                       |
 * Register                |
 * |                       |
 * HANDLER_QUEUED<-------  |
 * |                    |  |
 * Run                  |  |
 * |                    |  |
 * HANDLER_RUNNING      |  |
 * |           |        |  |
 * Finished    Register--  |
 * |                       |
 * -------------------------
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

#ifdef DEBUG
enum HANDLER_STATE { HANDLER_READY=1, HANDLER_QUEUED, HANDLER_RUNNING };
#endif

struct HANDLER_OBJECT;

typedef void (HANDLER_FUNCTION)( struct HANDLER_OBJECT * HandlerObj );

struct HANDLER_OBJECT
{
	union LINK Link;
	HANDLER_FUNCTION * Function;
	void * Context;

#ifdef DEBUG
	enum HANDLER_STATE State;
#endif
};

#ifdef DEBUG

void HandlerInit( struct HANDLER_OBJECT * handler );
void HandlerRegister( struct HANDLER_OBJECT * handler );
void HandlerRun(struct HANDLER_OBJECT * handler );
void HandlerFinish( struct HANDLER_OBJECT * handler );

#else

#define HandlerInit(handler) 
#define HandlerRegister(handler) 
#define HandlerRun(handler) 
#define HandlerFinish(handler)
#endif

#endif
