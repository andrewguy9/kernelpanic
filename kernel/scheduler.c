#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"timer.h"
#include"mutex.h"
#include"interrupt.h"
#include"context.h"

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

//Scheduler variables: Protected by SchedulerLock
struct LINKED_LIST Queue1;
struct LINKED_LIST Queue2;
struct LINKED_LIST * RunQueue;
struct LINKED_LIST * DoneQueue;

//Variables that need to be edited atomically.
struct POST_HANDLER_OBJECT SchedulerTimer;
BOOL QuantumExpired;

//Thread for idle loop ( the start up thread too )
struct THREAD IdleThread;

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
	ASSERT( aquired, 
			SCHEDULER_START_CRITICAL_MUTEX_NOT_AQUIRED,
			"Start Critical should always stop the scheduler");
}

/*
 * Re-enables the scheduler. 
 */
void SchedulerEndCritical()
{
	BOOL quantum;
	ASSERT( ContextIsCritical( ),
			SCHEDULER_END_CRITICAL_NOT_CRITICAL,
		   	"Critical section cannot start.");

	//Cant check QuantumExpired unless atomic.
	InterruptDisable();
	quantum = QuantumExpired;
	InterruptEnable();

	if( quantum )
	{//Quantum has expired while in crit section, fire manually.
		SchedulerForceSwitch();
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
	ASSERT( ContextIsCritical( ),
			SCHEDULER_FORCE_SWITCH_IS_CRITICAL,
			"Schedule will not run when in critical section");

	//This needs to be an atomic operation.
	InterruptDisable();	//TODO THIS COULD BE A REALY LONG ATOMIC SECTION, DOES THE SCHEDULE PHASE HAVE TO BE ATOMIC?

	//End the critical Section
	//so that we can schedule
	ContextUnlock( ); 

	Schedule(); //Schedule next thread manually...

	//Actually context switch.
	ContextSwitchIfNeeded();
}

/*
 * Takes a blocked thread and adds it back into active circulation. 
 */
void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( ContextIsCritical( ), 
			SCHEDULER_RESUME_THREAD_MUST_BE_CRIT,
			"Only run from critical section" );
	ASSERT( thread->State == THREAD_STATE_BLOCKED, 
			SCHEDULER_RESUME_THREAD_NOT_BLOCKED,
			"Thread not blocked" );
	ASSERT( thread != ContextGetActiveThread(),
			SCHEDULER_ACTIVE_THREAD_AWAKENED,
			"Active thread is by definition running");

	thread->State = THREAD_STATE_RUNNING;
	DEBUG_LED = DEBUG_LED | thread->Flag;

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
	struct THREAD * activeThread = ContextGetActiveThread();
	activeThread->State = THREAD_STATE_BLOCKED;
	DEBUG_LED = DEBUG_LED & ~activeThread->Flag;
}

void Schedule( void *arg )
{
	struct THREAD * activeThread;
	struct THREAD * nextThread = NULL; 
	//See if we are allowed to schedule (not in crit section)
	if( ContextLock( ) )
	{//We are allowed to schedule.
		activeThread = ContextGetActiveThread();
		//save old thread
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

		//restart the scheduler timer if its turned off.
		if( ! SchedulerTimer.Queued )
		{
			TimerRegister( &SchedulerTimer,
					nextThread->Priority,
					Schedule,
					NULL);
			QuantumExpired = FALSE;
		}

		//Set the next thread.
		ContextSetNextThread( nextThread );

		ContextUnlock( );
	}
	else
	{//we are not allowed to schedule.
		//mark the quantum as expired.
		InterruptDisable();
		QuantumExpired = TRUE;
		InterruptEnable();
	}
}//end Schedule

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
			Schedule,
			NULL	);
	QuantumExpired = FALSE;
	//Create a thread for idle loop.
	SchedulerCreateThread( &IdleThread, 1, NULL, 0, NULL, 0x01, FALSE );
	//Remove IdleThread from queues... TODO fix this HACK
	LinkedListInit( & Queue1 );
	LinkedListInit( & Queue2 );
	//Initialize context unit.
	ContextStartup( & IdleThread );
}

/*
 * Returns the locking context
 * from the active thread.
 */
struct LOCKING_CONTEXT * SchedulerGetLockingContext()
{
	struct THREAD * activeThread = ContextGetActiveThread();
	return &activeThread->LockingContext;
}

void SchedulerCreateThread( 
		struct THREAD * thread,
		unsigned char priority,
		char * stack,
		COUNT stackSize,
		THREAD_MAIN main,
		char flag,
		BOOL start)
{
	//Populate thread struct
	thread->Priority = priority;
	thread->Flag = flag;
	LockingInit( & thread->LockingContext );
	//Add thread to done queue.
	if( start )
	{
		thread->State = THREAD_STATE_RUNNING;
		DEBUG_LED = DEBUG_LED | flag;
		LinkedListEnqueue( &thread->Link.LinkedListLink, DoneQueue );
	}
	else
	{
		thread->State = THREAD_STATE_BLOCKED;
	}
	//initialize stack
	ContextInit( &thread->Stack,stack,stackSize,main);
}	
