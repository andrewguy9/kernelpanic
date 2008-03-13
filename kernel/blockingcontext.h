#ifndef BLOCKING_CONTEXT_H
#define BLOCKING_CONTEXT_H

#include"../utils/link.h"
#include"../utils/utils.h"

/*
State machine for a struct LOCKING_CONTEXT's state 
through a locking operation

random         checked
|              |  
Init()         Restart
|              |
ready,checked--checked------------------------------------
|                  |                   |                 |
start()            start()             start()           start()
startCrit          startcrit           startcrit         startcrit
|                  |                   |                 |
Acquire(Null)      Acquire(context)    Block(NULL)       Block(context)
checked            acquired            blocking          waiting
|                  |                   store thread      store context
|                  |                   |                 |
End()              End()               Switch(NULL)      Switch(context)
end crit           end crit            switch thread     switch thread
|                  *                   *                 *
|                  *                   Acquire(context)  Acquire(context)
|                  *                   checked           acquired
|                  *                   |                 |
|                  IsAcquired(conext)  |                 IsAcquired(context)
|                  checked             |                 checked
|                  |                   |                 |
----------------------------------------------------------
|
restart
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

enum RESOURCE_STATE 
{ 
	RESOURCE_SHARED, 
	RESOURCE_EXCLUSIVE 
};

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

/*
 * Procedure for acquiring or blocking on a lock using the locking framework.
 * LockingStart( ) - Enters a critical section
 * LockingBlock( ) or LockingAcquire(  ) - Perform locking operation
 * Calling lock stores away link in blocking case...
 * LockingSwitch( ) - does the obligatory SchedulerEndCritical() or SchedulerForceSwitch
 */

/*
 * Prodecure for unlocking a lock.
 * LockingStart( ) - Enters critical section
 * LockingAcquire(  ) for each unblocked context.
 * LockingEnd(  ) - ends critical section
 */

//start a locking operation
void LockingStart();
//end an unlock operation
void LockingEnd();
//end a lock operation
void LockingSwitch( struct LOCKING_CONTEXT * context );
//initialize a context
void LockingInit( struct LOCKING_CONTEXT * context );
//aquire a lock
void LockingAcquire( struct LOCKING_CONTEXT *context );
//block on a lock
union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * waitingContext );
//check to see if acquired.
BOOL LockingIsAcquired( struct LOCKING_CONTEXT * context );

#endif
