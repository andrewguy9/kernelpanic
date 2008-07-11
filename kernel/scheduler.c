#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"timer.h"
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

COUNT Schedule();
void SchedulePostHandler( void *arg );

//Scheduler variables: Protected by SchedulerLock
struct LINKED_LIST Queue1;
struct LINKED_LIST Queue2;
struct LINKED_LIST * RunQueue;
struct LINKED_LIST * DoneQueue;

struct THREAD * ActiveThread;
struct THREAD * NextThread;

//Variables that need to be edited atomically.
struct POST_HANDLER_OBJECT SchedulerTimer;
TIME QuantumEndTime;

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

	//Start Critical should always stop the scheduler
	ASSERT( aquired );
}

/*
 * Re-enables the scheduler. 
 */
void SchedulerEndCritical()
{
	TIME currentTime;
	COUNT priority;	
	BOOL needSwitch = FALSE;
	ASSERT( ContextIsCritical( ) );

	//Check if quantum has expired while in crit section
	currentTime = TimerGetTime();
	if( currentTime > QuantimEndTime )
		needSwitch = TRUE;

	//See if thread blocked itself.
	if(ActiveThread->State == THREAD_STATE_BLOCKED )
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
	ASSERT( thread != ActiveThread );

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
	ASSERT( ContextIsCritical() );

	ActiveThread->State = THREAD_STATE_BLOCKED;
	DEBUG_LED = DEBUG_LED & ~ActiveThread->Flag;
}

/*
 * Pick the next thread to run.
 * Changes links to store threads in lists.
 * Returns the priority of the next thread.
 */
COUNT Schedule()
{
	ASSERT( ContextIsCritical() );

	//save old thread unless its blocking or is the idle thread.
	if( ActiveThread != &IdleThread && 
			ActiveThread->State == THREAD_STATE_RUNNING)
	{
		LinkedListEnqueue( &ActiveThread->Link.LinkedListLink,
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
		NextThread = BASE_OBJECT( LinkedListPop( RunQueue ),
				struct THREAD,
				Link);
	}
	else
	{//there were no threads at all, use idle loop.
		NextThread = &IdleThread;
	}

	ContextSetNextContext( & NextThread->Stack );

	return NextThread->Priority;

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
			FALSE );

	ActiveThread = &IdleThread;
	NextThread = NULL;

	//Initialize context unit.
	ContextStartup( & IdleThread );
}

/*
 * Returns the locking context
 * from the active thread.
 */
struct LOCKING_CONTEXT * SchedulerGetLockingContext()
{
	ASSERT( ContextIsCritical() );

	return &ActiveThread->LockingContext;
}

void SchedulerThreadStartup()
{
	struct THREAD * thread;
	
	ASSERT( ContextIsCritical() );
	ASSERT( InterruptIsAtomic() );

	thread = ActiveThread;
	
	ContextUnlock();
	InterruptEnable();

	thread->Main( thread->Argument );
}

void SchedulerCreateThread( 
		struct THREAD * thread,
		unsigned char priority,
		char * stack,
		COUNT stackSize,
		THREAD_MAIN main,
		void * Argument,
		char flag,
		BOOL start)
{
	//Populate thread struct
	thread->Priority = priority;
	thread->Flag = flag;
	LockingInit( & thread->LockingContext, NULL );//TODO
	thread->Main = main;
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
	ContextInit( &thread->Stack,stack,stackSize,SchedulerThreadStartup);
}	
