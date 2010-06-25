#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"timer.h"
#include"interrupt.h"
#include"softinterrupt.h"
#include"critinterrupt.h"
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
 * reentrant compatable. However, today we use the Interrupt unit's counted
 * IRQ interface (Critical Section managed by the CritInterrupt IRQ).
 * We keep the Scheduler Critical interfaces because they have asserts
 * to protect against previously disallowed reentrancy. Someday we should
 * remove them when we become convinced that it is safe.
 */

void Schedule();
BOOL SchedulerTimerHandler( struct HANDLER_OBJECT * handler );
BOOL SchedulerCritHandler( struct HANDLER_OBJECT * handler );
void SchedulerNeedsSwitch();

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

//Thread for idle loop ( the start up thread too )
struct THREAD IdleThread;

//
//Private Helper Functions
//


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
 * Disables the scheduler so that the current stack will not be switched. This allows 
 * threads level atomicy without having to turn interrupts off.
 *
 * SchedulerStartCritical CANNOT be called recursively. 
 * TODO: We can we replaced with CritInterruptDisable
 */
void SchedulerStartCritical( )
{
	//This is only safe if we are in a thread or crit operation.
	ASSERT( !InterruptIsAtomic() && !SoftInterruptIsAtomic() );
	CritInterruptDisable();
}

/*
 * Re-enables the scheduler. 
 * TODO: We can replace this with calls to CritInterruptEnable
 */
void SchedulerEndCritical()
{
	ASSERT( CritInterruptIsAtomic() );
	CritInterruptEnable();
}

#ifdef DEBUG
/*
 * Returns 'TRUE' if the scheduler is turned off (is critical section).
 * returns 'FALSE' if the scheduler is turned on (not critical section).
 * Should be used in ASSERTs only.
 * TODO: We can move from this CritInterruptIsAtomic
 */
BOOL SchedulerIsCritical()
{
	return CritInterruptIsAtomic();
}
#endif //DEBUG

/*
 * Ends a critical section and forces an immediate context switch
 */
void  SchedulerForceSwitch()
{
	ASSERT( CritInterruptIsAtomic() );
	
	SchedulerNeedsSwitch();
	SchedulerEndCritical();
	ASSERT( !CritInterruptIsAtomic() );
}

/*
 * Takes a blocked thread and adds it back into active circulation. 
 */
void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( HalIsCritAtomic( ) );
	ASSERT( thread->State == THREAD_STATE_BLOCKED );

	thread->State = THREAD_STATE_RUNNING;

	LinkedListEnqueue( &thread->Link.LinkedListLink, &RunQueue );
}

BOOL SchedulerIsThreadDead( struct THREAD * thread )
{
	ASSERT( HalIsCritAtomic( ) );
	
	return thread->State == THREAD_STATE_DONE;
}

BOOL SchedulerIsThreadBlocked( struct THREAD * thread )
{
	ASSERT( HalIsCritAtomic( ) );
	
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
void SchedulerBlockThread( )
{
	ASSERT( HalIsCritAtomic() );

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
	ASSERT( ! InterruptIsAtomic() );

	//Update watchdog for both counters, since they have/are running.
	WatchdogNotify( ActiveThread->MachineContext.Flag );
	WatchdogNotify( NextThread->MachineContext.Flag );

	//Save off copy of the current state.
	oldThread = ActiveThread;
	newThread = NextThread;

	//Swap pointers so that when new threads
	//come up, ActiveThread will be pointing
	//at the right guy.
	ActiveThread = NextThread;
	NextThread = NULL;

	InterruptDisable();
	ContextSwitch(&oldThread->MachineContext, &newThread->MachineContext);
	InterruptEnable();

	ASSERT( ! InterruptIsAtomic() );
	ASSERT( CritInterruptIsAtomic() );
	
	//Release the lock so that the scheduler can run again.
	MutexUnlock( &SchedulerMutex );
}


/*
 * Pick the next thread to run.
 * Changes links to store threads in lists.
 * This function is NOT side effect free! If it is called, 
 * you MUST perform a context switch.
 */
void Schedule()
{
	ASSERT( HalIsCritAtomic() );

	//save old thread unless its blocking or is the idle thread.
	if( ActiveThread != &IdleThread && 
			ActiveThread->State == THREAD_STATE_RUNNING)
	{
		LinkedListEnqueue( &ActiveThread->Link.LinkedListLink,
				&RunQueue);
	}

	//Pick the next thread
	if( ! LinkedListIsEmpty( &RunQueue ) )
	{//there are threads waiting, run one
		NextThread = BASE_OBJECT( LinkedListPop( &RunQueue ),
				struct THREAD,
				Link);
	}
	else
	{//there were no threads at all, use idle loop.
		NextThread = &IdleThread;
	}
}//end Schedule

/*
 * Raises a context switch if needed
 */
void SchedulerNeedsSwitch()
{
	ASSERT( HalIsCritAtomic() );

	if( MutexLock( &SchedulerMutex ) )
	{
		//We acquired the mutex, so we know that a switch has been requested.
		CritInterruptRegisterHandler(
				& SchedulerCritObject,
				SchedulerCritHandler,
				NULL );
	}
}

/*
 * Called by the Timer as a PostInterruptHandler.
 * Will try to schedule. If we cant, then we 
 * mark the quantum as expired so that when the
 * critical section does end we can force a switch.
 */
BOOL SchedulerTimerHandler( struct HANDLER_OBJECT * handler )
{
	TIME currentTime = TimerGetTime();

	//Prevent the scheduler from running while we check the active thread.
	//Note that we can touch QuantumStartTime because we are a SoftISR.
	if( currentTime - QuantumStartTime > ActiveThread->Priority ) {
		SchedulerNeedsSwitch();
	} 
	else 
	{
		//We need to update the watchdog for the next thread.
		//printf("Watchdog notify on %ld\n", ActiveThread->MachineContext.Flag);
		WatchdogNotify( ActiveThread->MachineContext.Flag );
	}

	//Register timer fire again.
	TimerRegister( &SchedulerTimer,
			1,
			SchedulerTimerHandler,
			NULL );

	//We return false here because we have re-registered the timer.
	return FALSE;
}

/*
 * This function is the handler for the SchedulerCritObject. 
 * It gets registered when someone wants to switch threads.
 * It can be scheduled be queued by the scheduler timer firing 
 * or a thread yielding.
 */
BOOL SchedulerCritHandler( struct HANDLER_OBJECT * handler )
{
	//Select the next thread to run.
	//Re-queue the curring thread if he is giving up 
	//conrol volontarily. 
	Schedule();

	//Reset the quantum start time so that 
	//the timer will know when to wake the scheduler.
	//Note that we must disable SoftISRs while updating QuantumStartTime
	//so that it is consistant from the perspective of the timer.
	SoftInterruptDisable(); 
	QuantumStartTime = TimerGetTime();
	SoftInterruptEnable(); 

	//Switch away from the current thread to another.
	//This will release any held resources.
	SchedulerSwitch();

	//TODO: Returning FALSE here causes the work item to get leaked,
	//but prevents it from asserting when used. The only solution to 
	//this is to have new threads complete work items. this is good 
	//enough for now.
	return FALSE;
}

void SchedulerStartup()
{
	//Setup the hal to use the scheduler.
	HalContextStartup( SchedulerThreadStartup );
	
	//Initialize queues
	LinkedListInit( &RunQueue );
	//Initialize the timer
	HandlerInit( & SchedulerTimer );
	TimerRegister( & SchedulerTimer,
		   	0, 
			SchedulerTimerHandler,
			NULL	);
	
	//Initialize the crit handler
	MutexInit( &SchedulerMutex, FALSE );
	HandlerInit( &SchedulerCritObject );
	
	QuantumStartTime = 0;
	//Create a thread for idle loop.
	SchedulerCreateThread( &IdleThread, //Thread 
			1, //Priority
			NULL, //Stack
			0, //Stack Size
			NULL, //Main
			NULL, //Argument
			0x00, //Flag
			FALSE );//Start

	//Initialize context unit.
	ActiveThread = &IdleThread;
}

/*
 * Returns the locking context
 * from the active thread.
 */
struct LOCKING_CONTEXT * SchedulerGetLockingContext()
{
	ASSERT( HalIsCritAtomic() );

	ActiveThread = SchedulerGetActiveThread();
	return &ActiveThread->LockingContext;
}

void SchedulerThreadStartup( void )
{
	struct THREAD * thread;
	
	//Thread startup should occur in atomic section.
	//We are waking up from the trampoline, so we should not
	//be in a critical section, however, we know that the
	//scheduler was running when we were selected.
	//We need to make sure that we re-enable the scheduler.
	ASSERT( InterruptIsAtomic() );

	ASSERT( MutexIsLocked( &SchedulerMutex ) );
	MutexUnlock( &SchedulerMutex );
	
	//Get the thread.
	thread = SchedulerGetActiveThread();
	
	//We need to end atomic section before starting thread's main.
	InterruptEnable();
	ASSERT( !InterruptIsAtomic() );

	//Lets force a CritInterrupt drain so that we don't miss if there 
	//were elements in the queue when we bailed out.
	ASSERT( CritInterruptIsAtomic() );
	CritInterruptEnable();
	ASSERT( !CritInterruptIsAtomic() );

	//run the thread.
	thread->Main( thread->Argument );

	//Stop the thread
	SchedulerStartCritical();
	thread->State = THREAD_STATE_DONE;
	SchedulerForceSwitch();

	//We should never get here.
	//This function should not return.
	KernelPanic();
}

void SchedulerCreateThread( 
		struct THREAD * thread,
		unsigned char priority,
		char * stack,
		COUNT stackSize,
		THREAD_MAIN main,
		void * Argument,
		INDEX debugFlag,
		BOOL start)
{
	//Make sure data is valid
	ASSERT( debugFlag < 8 );
	ASSERT( HalIsCritAtomic() );

	//Populate thread struct
	thread->Priority = priority;
	LockingInit( & thread->LockingContext, SchedulerBlockOnLock, SchedulerWakeOnLock );
	thread->Main = main;
	thread->Argument = Argument;

	//initialize stack
	ContextInit( &(thread->MachineContext), stack, stackSize, SchedulerThreadStartup, debugFlag );

	//Add thread to queue.
	if( start )
	{
		thread->State = THREAD_STATE_RUNNING;
		LinkedListEnqueue( &thread->Link.LinkedListLink, &RunQueue );
	}
	else
	{
		thread->State = THREAD_STATE_BLOCKED;
	}
}	
