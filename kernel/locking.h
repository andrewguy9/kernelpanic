#ifndef LOCKING_H
#define LOCKING_H

#include"../utils/link.h"
#include"../utils/utils.h"
#include"blockingcontext.h"

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

enum LOCKING_STATE
{
	LOCKING_STATE_READY,
	LOCKING_STATE_BLOCKING,
	LOCKING_STATE_WAITING,
	LOCKING_STATE_ACQUIRED,
	LOCKING_STATE_CHECKED
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
//check to see if a context is in use.
BOOL LockingIsFree( struct LOCKING_CONTEXT * context );
#endif
