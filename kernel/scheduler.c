#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"timer.h"
#include"interrupt.h"
#include"critinterrupt.h"
#include"context.h"
#include"panic.h"
#include"mutex.h"

/*
 * Scheduler Unit:
 * The Scheduler unit has three tasks:
 * 1) Provide a thread scheduler which
 * picks when threads run.
 * 2) Provide a mechanism for a thread
 * to stop itself, and for others to wake it.
 * 3) Provide mechanism for thread level atomic 
 * or "critical sections".
 *
 * Scheduling next thread:
 * The actual Scheduling occurs in the Schedule
 * function. We must make sure that the Schedule 
 * function does pick a new thread while the 
 * system is in a thread critical section. 
 * We enforce this rule using a mutex. 
 * Threads entering a critical section
 * acquire the mutex, and then release it
 * at the end of their critical section. 
 *
 * Blocking Contexts:
 *TODO UPDATE COMMENT TO REFLECT NEW BLOCKING RULES
 * There are many resons to block a thread. Some
 * of them will need to save infomation about 
 * the thread's state so we will know when to wake it.
 * This information is stored in the thread's blocking context.
 *
 * Blocking contexts can only be used when a thread is blocked.
 * Only one unit can block a thread at a time.
 */

void Schedule();
BOOL SchedulerTimerHandler( struct HANDLER_OBJECT * handler );
BOOL SchedulerCritHandler( struct HANDLER_OBJECT * handler );
void SchedulerNeedsSwitch();

struct LINKED_LIST RunQueue;

//Variables that need to be edited atomically.
struct HANDLER_OBJECT SchedulerTimer;
TIME QuantumStartTime;

struct MUTEX SchedulerMutex;
struct HANDLER_OBJECT SchedulerCritObject;

//Thread for idle loop ( the start up thread too )
struct THREAD IdleThread;

//
//Private Helper Functions
//


struct THREAD * SchedulerGetActiveThread()
{
	return BASE_OBJECT( ContextGetContext(), struct THREAD, MachineContext );
}

void SchedulerBlockOnLock( struct LOCKING_CONTEXT * context )
{
	//
	//  We need to validate that the active thread owns the locking
	//  context provided.
	//
	
#ifdef DEBUG
	struct THREAD * activeThread = SchedulerGetActiveThread();

	//The context is embeded in a thread, extract it.
	struct THREAD * thread = BASE_OBJECT( context,
				struct THREAD,
				LockingContext);

	//Lets make sure that context belongs to the active thread
	ASSERT( thread == activeThread );
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
	CritInterruptDisable();
}

/*
 * Re-enables the scheduler. 
 * TODO: We can replace this with calls to CritInterruptEnable
 */
void SchedulerEndCritical()
{
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
	ASSERT( thread != SchedulerGetActiveThread() );

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
	struct THREAD * activeThread;
	ASSERT( HalIsCritAtomic() );

	activeThread = SchedulerGetActiveThread();

	activeThread->State = THREAD_STATE_BLOCKED;
	
	SchedulerNeedsSwitch();
}

/*
 * Does the actual call into the context unit to perform a switch.
 * Will also verify that the correct resources are held
 */
void SchedulerSwitch() 
{
	
	ASSERT( CritInterruptIsAtomic() );
	ASSERT( ! InterruptIsAtomic() );

	InterruptDisable();
	
	ContextSwitch();

	InterruptEnable();

	ASSERT( ! InterruptIsAtomic() );
	ASSERT( CritInterruptIsAtomic() );
	
	//Release the lock so that the scheduler can run again.
	MutexUnlock( &SchedulerMutex );
	
}


/*
 * Pick the next thread to run.
 * Changes links to store threads in lists.
 * Returns the priority of the next thread.
 */
void Schedule()
{
	struct THREAD * activeThread;
	struct THREAD * nextThread;

	ASSERT( HalIsCritAtomic() );

	activeThread = SchedulerGetActiveThread( );

	//save old thread unless its blocking or is the idle thread.
	if( activeThread != &IdleThread && 
			activeThread->State == THREAD_STATE_RUNNING)
	{
		LinkedListEnqueue( &activeThread->Link.LinkedListLink,
				&RunQueue);
	}

	//Pick the next thread
	if( ! LinkedListIsEmpty( &RunQueue ) )
	{//there are threads waiting, run one
		nextThread = BASE_OBJECT( LinkedListPop( &RunQueue ),
				struct THREAD,
				Link);
	}
	else
	{//there were no threads at all, use idle loop.
		nextThread = &IdleThread;
	}

	ContextSetNextContext( & nextThread->MachineContext );
}//end Schedule

/*
 * Raises a context switch if needed
 */
void SchedulerNeedsSwitch()
{
	ASSERT( HalIsCritAtomic() );

	if( MutexLock( &SchedulerMutex ) )
	{
		//The quantum has expired, so we should register an eviction if possible.
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
	struct THREAD * activeThread;
	
	//Prevent the scheduler from running while we check the active thread.
	//TODO I dont know if this will work, the active thread maybe in transition
	//when this timer fires. what does it mean to have the critInterruptDisabled 
	//in a timer when a timer runs at soft interupt time. 
	CritInterruptDisable(); 

	activeThread = SchedulerGetActiveThread();


	if( currentTime - QuantumStartTime > activeThread->Priority ) {
		SchedulerNeedsSwitch();
	}

	CritInterruptEnable();

	//Register timer fire again.
	TimerRegister( &SchedulerTimer,
			1,
			SchedulerTimerHandler,
			NULL);

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

	//Reset the quantim start time so that 
	//the timer will know when to wake the scheduler.
	InterruptDisable();
	QuantumStartTime = TimerGetTime();
	InterruptEnable();

	//Switch away from the current thread to another.
	//This will release any held resources.
	SchedulerSwitch();

	//TODO: I think returning false here will help.
	return FALSE;
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
	ContextSetActiveContext( & IdleThread.MachineContext );
}

/*
 * Returns the locking context
 * from the active thread.
 */
struct LOCKING_CONTEXT * SchedulerGetLockingContext()
{
	struct THREAD * activeThread;

	ASSERT( HalIsCritAtomic() );

	activeThread = SchedulerGetActiveThread();
	return &activeThread->LockingContext;
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
