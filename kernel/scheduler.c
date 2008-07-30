#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"timer.h"
#include"interrupt.h"
#include"context.h"
#include"panic.h"

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
 * aquire the mutex, and then release it
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

COUNT Schedule();
void SchedulePostHandler( void *arg );

//Scheduler variables: Protected by SchedulerLock
struct LINKED_LIST Queue1;
struct LINKED_LIST Queue2;
struct LINKED_LIST * RunQueue;
struct LINKED_LIST * DoneQueue;

//Variables that need to be edited atomically.
struct POST_HANDLER_OBJECT SchedulerTimer;
TIME QuantumEndTime;

//Thread for idle loop ( the start up thread too )
struct THREAD IdleThread;

//
//Private Helper Functions
//

#define SchedulerGetActiveThread( ) ( BASE_OBJECT( ContextGetStack(), struct THREAD, Stack ) )

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

	//blocking calls do not require notification. 
	//Set locking state to ready.
	context->State = LOCKING_STATE_READY;
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
 */
void SchedulerStartCritical( )
{
	BOOL aquired = ContextLock( );

	//Start Critical should always stop the scheduler
	ASSERT( aquired );
}

/*
 * Re-enables the scheduler. 
 */
void SchedulerEndCritical()
{
	TIME currentTime;
	struct THREAD * activeThread;
	COUNT priority;	
	BOOL needSwitch = FALSE;
	ASSERT( ContextIsCritical( ) );

	//Check if quantum has expired while in crit section
	currentTime = TimerGetTime();
	if( currentTime > QuantumEndTime )
		needSwitch = TRUE;

	//See if thread blocked itself.
	activeThread = SchedulerGetActiveThread();
	if(activeThread->State == THREAD_STATE_BLOCKED )
		needSwitch = TRUE;

	if( needSwitch )
	{
		//Pick next thread
		priority = Schedule();

		//set new end time.
		QuantumEndTime = currentTime+priority;

		//Switch threads!
		InterruptDisable();
		ContextSwitchIfNeeded();
		InterruptEnable();
	}
	else
	{//Quantum has not expired, so we'll just end the critical section. 
		ContextUnlock( );
	}
}

/*
 * Returns 'TRUE' if the scheduler is turned off (is critical section).
 * returns 'FALSE' if the scheduler is turned on (not critical section).
 * Should be used in ASSERTs only.
 */
BOOL SchedulerIsCritical()
{
	return ContextIsCritical( );
}

/*
 * Ends a critical section and forces an immediate context switch
 */
void  
SchedulerForceSwitch()
{
	TIME currentTime;
	COUNT priority;

	ASSERT( ContextIsCritical( ) );

	currentTime = TimerGetTime();

	//Pick next thread to run.
	priority = Schedule(); 
	
	//Set end of quantum.
	QuantumEndTime = currentTime + priority;

	//Actually context switch.
	InterruptDisable();
	ContextSwitchIfNeeded();
	InterruptEnable();
}

/*
 * Takes a blocked thread and adds it back into active circulation. 
 */
void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( ContextIsCritical( ) );
	ASSERT( thread->State == THREAD_STATE_BLOCKED );
	ASSERT( thread != SchedulerGetActiveThread() );

	thread->State = THREAD_STATE_RUNNING;
	HalSetDebugLedFlag( thread->Flag );

	LinkedListEnqueue( &thread->Link.LinkedListLink, DoneQueue );
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
	ASSERT( ContextIsCritical() );

	activeThread = SchedulerGetActiveThread();

	activeThread->State = THREAD_STATE_BLOCKED;
	HalClearDebugLedFlag( activeThread->Flag );
}

/*
 * Pick the next thread to run.
 * Changes links to store threads in lists.
 * Returns the priority of the next thread.
 */
COUNT Schedule()
{
	struct THREAD * activeThread;
	struct THREAD * nextThread;

	ASSERT( ContextIsCritical() );

	activeThread = SchedulerGetActiveThread( );

	//save old thread unless its blocking or is the idle thread.
	if( activeThread != &IdleThread && 
			activeThread->State == THREAD_STATE_RUNNING)
	{
		LinkedListEnqueue( &activeThread->Link.LinkedListLink,
				DoneQueue);
	}

	if( LinkedListIsEmpty( RunQueue ) )
	{
		//There are no threads in run queue.
		//This is a sign we have run through
		//all threads, so well pull from done
		//queue now
		struct LINKED_LIST * temp = RunQueue;
		RunQueue = DoneQueue;
		DoneQueue = temp;
	}

	//Pick the next thread
	if( ! LinkedListIsEmpty( RunQueue ) )
	{//there are threads waiting, run one
		nextThread = BASE_OBJECT( LinkedListPop( RunQueue ),
				struct THREAD,
				Link);
	}
	else
	{//there were no threads at all, use idle loop.
		nextThread = &IdleThread;
	}

	ContextSetNextContext( & nextThread->Stack );

	return nextThread->Priority;

}//end Schedule

/*
 * Called by the Timer as a PostInterruptHandler.
 * Will try to schedule. If we cant, then we 
 * mark the quantum as expired so that when the
 * critical section does end we can force a switch.
 */
void SchedulePostHandler( void *arg )
{
	TIME currentTime;
	COUNT priority;

	//We are going to try to switch to a new thread.
	//Inorder to to this we must acquire a critical section.
	if( ContextLock( ) )
	{
		//We were able to enter a critical secion!
		//Now we can schedule a different thread to run.
		
		//check and see if quanum has expired.
		currentTime = TimerGetTime();

		if( currentTime >= QuantumEndTime )
		{
			//quantum is over, so pick another thread.
			priority = Schedule();

			//Calculate the time to end our quantum.
			QuantumEndTime = currentTime+priority;

			//NOTE: We leak the critical section here because InterruptEnd 
			//will end it when he does the context switch.
		}
		else
		{
			//quantim is not over, so end our critical section
			ContextUnlock();
		}
	}

	//Register timer fire again.
	TimerRegister( &SchedulerTimer,
			1,
			SchedulePostHandler,
			NULL);
}

void SchedulerStartup()
{
	//Initialize queues
	LinkedListInit( & Queue1 );
	LinkedListInit( & Queue2 );
	RunQueue = & Queue1;
	DoneQueue = & Queue2;
	//Initialize the timer
	TimerRegister( & SchedulerTimer,
		   	0, 
			SchedulePostHandler,
			NULL	);

	QuantumEndTime = 0;
	//Create a thread for idle loop.
	SchedulerCreateThread( &IdleThread, //Thread 
			1, //Priority
			NULL, //Stack
			0, //Stack Size
			NULL, //Main
			NULL, //Argument
			0x01, //Flag
			FALSE );//Start

	//Initialize context unit.
	ContextStartup( & IdleThread.Stack );
}

/*
 * Returns the locking context
 * from the active thread.
 */
struct LOCKING_CONTEXT * SchedulerGetLockingContext()
{
	struct THREAD * activeThread;

	ASSERT( ContextIsCritical() );

	activeThread = SchedulerGetActiveThread();
	return &activeThread->LockingContext;
}

void SchedulerThreadStartup()
{
	struct THREAD * thread;
	struct THREAD * activeThread;
	
	ASSERT( ContextIsCritical() );
	ASSERT( InterruptIsAtomic() );

	activeThread = SchedulerGetActiveThread();
	thread = activeThread;
	
	ContextUnlock();
	InterruptEnable();

	thread->Main( thread->Argument );

	KernelPanic();//TODO we should support threads ending... just not now.
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

	//Populate thread struct
	thread->Priority = priority;
	thread->Flag = debugFlag;
	LockingInit( & thread->LockingContext, SchedulerBlockOnLock, SchedulerWakeOnLock );
	thread->Main = main;
	thread->Argument = Argument;
	//Add thread to done queue.
	if( start )
	{
		thread->State = THREAD_STATE_RUNNING;
		HalSetDebugLedFlag( debugFlag );
		LinkedListEnqueue( &thread->Link.LinkedListLink, DoneQueue );
	}
	else
	{
		thread->State = THREAD_STATE_BLOCKED;
	}
	//initialize stack
	ContextInit( &thread->Stack,stack,stackSize,SchedulerThreadStartup);
}	
