#include"scheduler.h"
#include"utils/utils.h"
#include"utils/linkedlist.h"
#include"time.h"
#include"timer.h"
#include"softinterrupt.h"
#include"context.h"
#include"panic.h"
#include"mutex.h"
#include"watchdog.h"

/*
 * Scheduler Unit:
 * The Scheduler unit has three tasks:
 * 1) Provide a thread scheduler which
 * picks when threads run.
 *
 * 2) Provide thread APIs.
 *
 * 3) Provide mechanism for thread level atomic
 * or "critical sections".
 *
 * Scheduling next thread:
 * Selection of the next thread to run is performed by the Schedule()
 * function. Calling SchedulerSwitch() causes the kernel to flip to the
 * next thread's stack of execution. Selection of the next thread can
 * be triggered by either a thread yilding control or having his quantum
 * expire. The SchedulerCritObject is a crit handler used to perform
 * stack switches. The SchedulerMutex is used serialize attempts at
 * switching threads.
 *
 * Thread APIs:
 * Allow threads to be spawned, block themselves, etc.
 *
 * CriticalSections:
 * Once upon a time, critical sections were maintained with a mutex.
 * This meant that the Scheduler Critical Section APIs were not
 * reentrant compatable. However, today we use the ISR unit's counted
 * IRQ interface (Critical Section managed by the CritInterrupt IRQ).
 * We keep the Scheduler Critical interfaces because they have asserts
 * to protect against previously disallowed reentrancy.
 * TODO Someday we should remove them when we become convinced that it is safe.
 */

void Schedule();
HANDLER_FUNCTION SchedulerTimerHandler;
HANDLER_FUNCTION SchedulerCritHandler;
void SchedulerNeedsSwitch();
STACK_INIT_ROUTINE SchedulerThreadStartup;
HANDLER_COMPLETE_FUNCTION PostCritHandler;

//NOTE: ActiveThread, NextThread, RunQueue are protected by IRQ_LEVEL_CRIT.
//NOTE: QuantumStartTime is used by the SchedulerTimer, so update to him need to be IRQ_LEVEL_SOFT (Timers run at soft, not IRQ_LEVEL_TIMER).

//Pointers to the currnet and next thread.
struct THREAD * ActiveThread;
struct THREAD * NextThread;

//List of threads which are eligable to run.
struct LINKED_LIST RunQueue;

//HANDLER used as a timer for the thread eviction process.
struct HANDLER_OBJECT SchedulerTimer;

//The Time at which ActiveThread was selected to run.
//QuantimStartTime should be protected at the SoftInterrupt IRQ
//so that the timer can evaluate it.
volatile TIME QuantumStartTime;

//HANDLER used to perform thread scheduling.
//SchedulerMutex is used to restrict access to SchedulerCritObject,
//because it can be queued by a thread calling yield or the
//periodic thread eviction process.
struct HANDLER_OBJECT SchedulerCritObject;
struct MUTEX SchedulerMutex;

//Count of the threads which have not returned.
//Used for shutdown tracking.

COUNT RunningThreads;

//Track if we are shutting down.
_Bool Shutdown;

void PostCritHandler(struct HANDLER_OBJECT * obj )
{
        ASSERT(obj == &SchedulerCritObject);

        ASSERT(MutexIsLocked(&SchedulerMutex));
        MutexUnlock(&SchedulerMutex);
}

//Thread for idle loop ( the start up thread too )
struct THREAD IdleThread;

//
//Private Helper Functions
//


/*
 * Returns a pointer to the current thread. Should only
 * be called by a thread, never in interrupt.
 */
//TODO CALLED ONLY FROM SLEEP UNIT, CAN THIS BE INTERNAL?
struct THREAD * SchedulerGetActiveThread()
{
        return ActiveThread;
}

void SchedulerBlockOnLock( struct LOCKING_CONTEXT * context )
{
        //
        //  We need to validate that the active thread owns the locking
        //  context provided.
        //

#ifdef DEBUG
        //The context is embeded in a thread, extract it.
        struct THREAD * thread = BASE_OBJECT( context,
                        struct THREAD,
                        LockingContext);

        //Lets make sure that context belongs to the active thread
        ASSERT( thread == ActiveThread );
#endif

        //We need to block the thread so it is not rescheduled
        //until the lock is acquired.

        SchedulerBlockThread( );

}

void SchedulerWakeOnLock( struct LOCKING_CONTEXT * context )
{
        //The context is embeded in a thread, extract it.
        struct THREAD * thread = BASE_OBJECT( context,
                        struct THREAD,
                        LockingContext);

        switch( context->State )
        {
                case LOCKING_STATE_READY:
                        //we acquired the lock right away.
                        //Since threads no not require notification, mark as ready.
                        context->State = LOCKING_STATE_READY;
                        break;

                case LOCKING_STATE_BLOCKING:
                        //we acquired the lock after blocking.
                        //mark as acquired and wake thread.
                        context->State = LOCKING_STATE_READY;
                        SchedulerResumeThread( thread );
                        break;

                case LOCKING_STATE_ACQUIRED:
                default:
                        KernelPanic();
                        break;
        }
}

//
//Public Functions
//

/*
 * Ends a critical section and forces an immediate context switch
 */
void SchedulerForceSwitch()
{
        ASSERT( CritInterruptIsAtomic() );

        SchedulerNeedsSwitch();
        SchedulerEndCritical();
        ASSERT( !CritInterruptIsAtomic() );
}

/*
 * Takes a blocked thread and adds it back into active circulation.
 */
//TODO THIS IS USED FOR LOCKING WAKE. CAN IT BE INTERNAL FUNCTION?
void SchedulerResumeThread( struct THREAD * thread )
{
        ASSERT( CritInterruptIsAtomic() );
        ASSERT( thread->State == THREAD_STATE_BLOCKED );

        thread->State = THREAD_STATE_RUNNING;

        LinkedListEnqueue( &thread->Link.LinkedListLink, &RunQueue );
}

_Bool SchedulerIsThreadDead( struct THREAD * thread )
{
        ASSERT( CritInterruptIsAtomic() );

        return thread->State == THREAD_STATE_DONE;
}

_Bool SchedulerIsThreadBlocked( struct THREAD * thread )
{
        ASSERT( CritInterruptIsAtomic() );

        return thread->State == THREAD_STATE_BLOCKED;
}

/*
 * A thread can call SchedulerBlockThread to prevent it from being
 * added back into the thread queue when its switched out.
 *
 * Must be called in critical section.
 *
 * Threads which call SchedulerBlockThread should have some
 * mechanism to wake the thread later.
 */
//TODO USED IN SLEEP AND LOCK MANAGEMENT. COULD BE INTENRAL FUNCTION?
void SchedulerBlockThread( )
{
        ASSERT( CritInterruptIsAtomic() );

        ActiveThread->State = THREAD_STATE_BLOCKED;

        SchedulerNeedsSwitch();
}

/*
 * Does the actual call into the context unit to perform a switch.
 * Will also verify that the correct resources are held
 */
void SchedulerSwitch()
{
        struct THREAD * oldThread;
        struct THREAD * newThread;

        ASSERT( CritInterruptIsAtomic() );
        //TODO WHAT I'M ACTUALLY TRYING TO DO IS MAKE SURE THAT THE NUMBER OF DISABLED COUNTS IS EQUAL
        //TODO ON BOTH THREADS. THIS WAY THE KERNEL KNOWS THAT ENABLED CALLS WONT BE MISSED.
        //TODO TODAY WE GUARANTEE THIS BY BEING REALLY RIDGID ABOUT WHAT THE COUNT HAS TO BE.
        //TODO IT WOULD BE NICE TO BE MORE FLEXABLE HERE.
        ASSERT( ! TimerIsAtomic() );

        //Save off copy of the current state.
        oldThread = ActiveThread;
        newThread = NextThread;

        //Swap pointers so that when new threads
        //come up, ActiveThread will be pointing
        //at the right guy.
        ActiveThread = NextThread;
        NextThread = NULL;

        ContextSwitch(&oldThread->Stack, &newThread->Stack);

        ASSERT( ! TimerIsAtomic() );
        ASSERT( CritInterruptIsAtomic() );//TODO SEE NOTE ABOVE.
}


/*
 * Pick the next thread to run.
 * Changes links to store threads in lists.
 * This function is NOT side effect free! If it is called,
 * you MUST perform a context switch.
 */
void Schedule()
{
        ASSERT( CritInterruptIsAtomic() );

        //save old thread unless its blocking or is the idle thread.
        if ( ActiveThread != &IdleThread &&
                        ActiveThread->State == THREAD_STATE_RUNNING) {
                LinkedListEnqueue( &ActiveThread->Link.LinkedListLink,
                                &RunQueue);
        }

        //Pick the next thread
        if ( ! LinkedListIsEmpty( &RunQueue ) ) {
                //there are threads waiting, run one
                NextThread = BASE_OBJECT( LinkedListPop( &RunQueue ),
                                struct THREAD,
                                Link);
        } else {
                //there were no threads at all, use idle loop.
                NextThread = &IdleThread;
        }
}//end Schedule

/*
 * Raises a context switch if needed
 */
void SchedulerNeedsSwitch()
{
        if ( MutexLock( &SchedulerMutex ) ) {
                HandlerInit( &SchedulerCritObject );
                HandlerCompetion(&SchedulerCritObject, PostCritHandler);
                //We acquired the mutex, so we know that a switch has been requested.
                CritInterruptRegisterHandler(
                                & SchedulerCritObject,
                                SchedulerCritHandler,
                                NULL );
        } else {
                //We failed to acquire the mutex, so we know that someone
                //else is already evicting the thread.
        }
}

/*
 * Called by the Timer as a PostInterruptHandler.
 * Will try to schedule. If we cant, then we
 * mark the quantum as expired so that when the
 * critical section does end we can force a switch.
 */
_Bool SchedulerTimerHandler( struct HANDLER_OBJECT * handler )
{
        TIME currentTime = TimeGet();

        //Only the global SchedulerTimer should invoke this callback.
        ASSERT( handler == &SchedulerTimer );

        //Prevent the scheduler from running while we check the active thread.
        //Note that we can touch QuantumStartTime because we are a SoftISR.
        if ( currentTime - QuantumStartTime > ActiveThread->Priority ) {
                SchedulerNeedsSwitch();
        }

        //Register timer fire again.
        TimerRegister( &SchedulerTimer,
                        1,
                        SchedulerTimerHandler,
                        NULL );

        //We return false here because we have re-registered the timer.
        return false;
}

/*
 * This function is the handler for the SchedulerCritObject.
 * It gets registered when someone wants to switch threads.
 * It can be scheduled be queued by the scheduler timer firing
 * or a thread yielding.
 */
_Bool SchedulerCritHandler( struct HANDLER_OBJECT * handler )
{
        //Only the global SchedulerCritObject should invoke this callback.
        ASSERT( handler == &SchedulerCritObject );

        //Select the next thread to run.
        //Re-queue the curring thread if he is giving up
        //conrol volontarily.
        Schedule();

        //Reset the quantum start time so that
        //the timer will know when to wake the scheduler.
        //Note that we must disable SoftISRs while updating QuantumStartTime
        //so that it is consistant from the perspective of the timer.
        //You might think that there is a race between SoftInterruptEnable() and SchedulerSwitch,
        //and you would be right, but this doesn't matter because the timer will fail to acquire
        //the SchedulerMutex. Worst case we will let a thread run 1 ms. too long.
        SoftInterruptDisable();
        QuantumStartTime = TimeGet();
        SoftInterruptEnable();

        //Switch away from the current thread to another.
        //This will release any held resources.
        SchedulerSwitch();

        return true;
}

void SchedulerStartup()
{
        //Initialize queues
        LinkedListInit( &RunQueue );

        //Initialize the timer
        HandlerInit( & SchedulerTimer );
        TimerRegister( & SchedulerTimer,
                        0,
                        SchedulerTimerHandler,
                        NULL);

        //Initialize the crit handler
        MutexInit( &SchedulerMutex, false );

        QuantumStartTime = TimeGet();

        RunningThreads = 0;
        Shutdown = false;

        //Create a thread for idle loop.
        SchedulerCreateThread( &IdleThread, //Thread
                        1, //Priority
                        NULL, //Stack
                        0, //Stack Size
                        NULL, //Main
                        NULL, //Argument
                        false );//Start

        //Initialize context unit.
        ActiveThread = &IdleThread;
}

void SchedulerShutdown( ) {
        SchedulerStartCritical();
        Shutdown = true;
        SchedulerEndCritical();
}

_Bool SchedulerIsShuttingDown( ) {
        SchedulerStartCritical();
        _Bool result = Shutdown;
        SchedulerEndCritical();
        return result;
}

/*
 * Returns the locking context
 * from the active thread.
 */
struct LOCKING_CONTEXT * SchedulerGetLockingContext()
{
        ASSERT( HalIsIrqAtomic(IRQ_LEVEL_CRIT) );

        ActiveThread = SchedulerGetActiveThread();
        return &ActiveThread->LockingContext;
}

void SchedulerThreadStartup( void * arg )
{
        struct THREAD * thread;

        //This is the first function called when context
        //switching into a newly created thread.
        //We are waking up from the new stack 'trampoline',
        //so we know that we came from a context switch.
        //This means we should release the scheduler resources
        //which were held across the context switch,
        //namely the SchedulerCritObject, and the SchedulerMutex.

        thread = arg;

        ASSERT (SchedulerGetActiveThread() == thread);

        //We should be in a critical section because we context switched here,
        //leaking the raise.
        ASSERT( CritInterruptIsAtomic() );

        //Now we will release the scheduler objects to allow the scheduler
        //to be consistant again.
        ASSERT( MutexIsLocked( &SchedulerMutex ) );
        HandlerFinish( &SchedulerCritObject );

        //Now we can end the critical section. Now the ContextSwitch is complete.
        CritInterruptEnable();
        ASSERT( !CritInterruptIsAtomic() );

        //run the thread.
        thread->Result = thread->Main( thread->Argument );

        //Release threads waiting to join this thread.
        ResourceUnlockExclusive(&thread->ResultLock);

        //Re-acquire the result structure, so that thread can die.
        ResourceLockExclusive(&thread->ResultLock, NULL);

        //The new thread's main returned!
        SchedulerStartCritical();
        thread->State = THREAD_STATE_DONE;

        //Check to see if we have shutdown the last active thread.
        RunningThreads--;
        if (Shutdown && RunningThreads == 1) {
          //Shutting down and only idle thread left.
          HalShutdownNow();
        }

        //We must ensure that we do not return from the trampoline,
        //so force the kernel to switch to another thread. Also move the
        //thread into THREAD_STATE_DONE so that he is never rescheduled.
        SchedulerForceSwitch();

        //We should never get here.
        //This function MUST not return.
        KernelPanic();
}

void SchedulerCreateThread(
                struct THREAD * thread,
                unsigned char priority,
                char * stack,
                COUNT stackSize,
                THREAD_MAIN main,
                void * Argument,
                _Bool start)
{
        ASSERT( HalIsIrqAtomic(IRQ_LEVEL_CRIT) );

        RunningThreads++;

        //Populate thread struct
        thread->Priority = priority;
        LockingInit( & thread->LockingContext, SchedulerBlockOnLock, SchedulerWakeOnLock );
        ResourceInit(& thread->ResultLock, RESOURCE_EXCLUSIVE);
        thread->Main = main;
        thread->Argument = Argument;
        thread->Result = NULL;

        //initialize stack
        ContextInit( &thread->Stack, stack, stackSize, SchedulerThreadStartup, thread );

        //Add thread to queue.
        if ( start ) {
                thread->State = THREAD_STATE_RUNNING;
                LinkedListEnqueue( &thread->Link.LinkedListLink, &RunQueue );
        } else {
                thread->State = THREAD_STATE_BLOCKED;
        }
}

void * SchedulerJoinThread(struct THREAD * thread) {
  ResourceLockShared( & thread->ResultLock, NULL);
  return thread->Result;
}

void SchedulerReleaseThread(struct THREAD * thread) {
  ResourceUnlockShared( & thread->ResultLock);
}
