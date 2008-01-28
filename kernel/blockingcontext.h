#ifndef BLOCKING_CONTEXT_H
#define BLOCKING_CONTEXT_H

#include"resource.h"

/*
random
|
Init()
|
ready, checked--------------------------------------------
|                  |                   |                 |
acquire(null)      acquire(context)    block(null)       block(context)
|                  |                   |                 |
checked            acquired            blocking          waiting
|                  |                   |                 |
|                  IsAcquired()        acquire(null)     acquire(context)
|                  |                   |                 |
|                  checked             checked           acquired
|                  |                   |                 |
|                  |                   |                 IsAcquired()
|                  |                   |                 |
|                  |                   |                 checked
|                  |                   |                 |
acquire(), or block()-------------------------------------
*/

enum LOCKING_STATE
{
	LOCKING_STATE_READY,
	LOCKING_STATE_BLOCKING,
	LOCKING_STATE_WAITING,
	LOCKING_STATE_ACQUIRED,
	LOCKING_STATE_CHECKED
};

/*
 * A lock may have to store information about how the thread wanted to acquire the lock.
 * This is where that data should be stored. However it is important to keep this as small
 * as possible.
 */

union BLOCKING_CONTEXT 
{
	enum RESOURCE_STATE ResourceWaitState;
};

/*
 * Structure to unify storage of requests.
 */
struct LOCKING_CONTEXT
{
	enum LOCKING_STATE State;
	union LINK Link;
	union BLOCKING_CONTEXT BlockingContext;
};

void LockingInit( struct LOCKING_CONTEXT * context );
void LockingAcquire( struct LOCKING_CONTEXT *context );
union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * waitingContext );
BOOL LockingIsAcquired( struct LOCKING_CONTEXT * context );

#endif
