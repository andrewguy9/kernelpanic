#ifndef LOCKING_H
#define LOCKING_H

#include"utils/link.h"
#include"utils/utils.h"
#include"blockingcontext.h"

/*
State machine for a struct LOCKING_CONTEXT's state
through a locking operation...

THREAD(ACQUIRED)    PLAIN(ACQIRED)    THREAD(BLOCK)     PLAIN(BLOCK)       WORKER(ACUQIRED)   WORKER(BLOCK)
[random]      [ready]
*             *
Init()        *
*             *
[ready]******************************************************************************************
*                  *                   *                 *                   *                  *
Start()            Start()             Start()           Start()             Start()            Start()
start critical     start critical      start critical    start critical      start critical     start critical
|                  |                   |                 |                   |                  |
Acquire()          Acquire()           Block()           Block()             Acquire()          Block()
"set state"        "set state"         "block thread"    "enter wait"        "set state"        "pend worker"
[ready]            [acquired]          [blocking]        [blocking]          [acquried]         [blocking]
|                  |                   |                 |                   |                  |
|                  |                   {store context}   {store context}     |                  {store context}
|                  |                   |                 |                   |                  |
End()              End()               End()             End()               End()              End()
end critical       end critical        end critical      end critical        end critical       end critical
*                  *                   *                 *                   *                  *
*                  *                   Start()           Start()             *                  Start()
*                  *                   |                 |                   *                  |
*                  *                   Acquire()         Acquire()           *                  Acquire()
*                  *                   "wake thread"     "end wait state"    *                  "wake work item"
*                  *                   [ready]           [acquired]          *                  [ready]
*                  *                   |                 |                   *                  |
*                  *                   End()             End()               *                  End()
*                  *                   *                 *                   *                  *
*                  IsAcquired()        *                 IsAcquired()        IsAcquired()       IsAcquired()
*                  [ready]             *                 [ready]             [ready]            [ready]
*                  *                   *                 *                   *                  *
*************************************************************************************************
|
restart

Key:
[locking context state]
{action that lock should take}
LockingCall()
"action from a wake or wait function"
| = The prisitine execution of a well defined critical section.
* = The piss stained waters in uncontrolled code section.
*/

/*
 * Procedure for acquiring or blocking on a lock using the locking framework.
 * LockingStart( ) - Enters a critical section
 * LockingBlock( ) or LockingAcquire(  ) - Perform locking operation
 * Caller lock stores away link in blocking case...
 * End( ) - does the obligatory SchedulerEndCritical which may context switch.
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
        LOCKING_STATE_ACQUIRED
};

//Prototype for wake functions.
struct LOCKING_CONTEXT;
typedef void WAKE_FUNCTION( struct LOCKING_CONTEXT * context );
typedef void BLOCK_FUNCTION( struct LOCKING_CONTEXT * context );

/*
 * Structure to unify storage of requests.
 */
struct LOCKING_CONTEXT
{
        enum LOCKING_STATE State;
        union LINK Link;
        union BLOCKING_CONTEXT BlockingContext;
        BLOCK_FUNCTION * BlockFunction;
        WAKE_FUNCTION * WakeFunction;
};

//start a locking operation
void LockingStart();
//initialize a context
void LockingInit( struct LOCKING_CONTEXT * context, BLOCK_FUNCTION * block, WAKE_FUNCTION * wake );
//aquire a lock
void LockingAcquire( struct LOCKING_CONTEXT *context );
//block on a lock
union LINK * LockingBlock( union BLOCKING_CONTEXT * blockingInfo, struct LOCKING_CONTEXT * waitingContext );
//check to see if acquired.
_Bool LockingIsAcquired( struct LOCKING_CONTEXT * context );
//check to see if a context is in use.
_Bool LockingIsFree( struct LOCKING_CONTEXT * context );
//end a locking sessoin.
void LockingEnd();

//
//Wake and Block functions for NonBlocking Consumers
//

void LockingWakeNonBlocking( struct LOCKING_CONTEXT * context );
void LockingBlockNonBlocking( struct LOCKING_CONTEXT * context );

#endif
