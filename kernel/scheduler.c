#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"timer.h"
#include"mutex.h"
#include"interrupt.h"
#include"hal.h"

/*
 * Scheduler Unit:
 * The Scheduler unit has three tasks:
 * 1) Provide a thread scheduler which
 * picks when threads run.
 * 2) Provide a mechanism for a thread
 * to stop itself, and for others to wake it.
 * 3) Provide mechanism to prevent the sceduler
 * from splitting thread level atomic
 * operations.
 *
 * Scheduling next thread:
 * The actual Scheduling occurs in the Schedule
 * function. We must make sure that the Schedule 
 * function does pick a new thread while the 
 * system is in a thread critical section. 
 * We enforce this rule using a mutex, 
 * threads entering a critical section
 * aquire the mutex, and then release it
 * at the end of their critical section. 
 *
 * Resuming and Blocking threads:
 * Resuming threads will require locking the waiting
 * thread 
 */

//Used to mark critical sections...
struct MUTEX SchedulerLock;

//Scheduler variables: Protected by SchedulerLock
struct LINKED_LIST Queue1;
struct LINKED_LIST Queue2;
struct LINKED_LIST * RunQueue;
struct LINKED_LIST * DoneQueue;

struct THREAD * ActiveThread;
struct THREAD * NextThread;


//Variables that need to be edited atomically.
struct HANDLER_OBJECT SchedulerTimer;
BOOL QuantumExpired;

//Thread for idle loop ( the start of thread too )
struct THREAD IdleThread;

/*
 * Thread function to start a 
 * critical section. 
 */
void SchedulerStartCritical( )
{
	BOOL aquired = MutexLock( & SchedulerLock );
	ASSERT( aquired, 
			SCHEDULER_START_CRITICAL_MUTEX_NOT_AQUIRED,
			"Start Critical should always stop the scheduler");
}

void SchedulerEndCritical()
{
	BOOL quantumHasExpired;
	ASSERT( MutexIsLocked( & SchedulerLock ),
			SCHEDULER_END_CRITICAL_NOT_CRITICAL,
		   	"Critical section cannot start.");

	//Cant check QuantumExpired unless atomic.
	InterruptDisable();
	
	if( QuantumExpired )
	{//Quantum has expired while in crit section, fire manually.
		SchedulerForceSwitch();
	}
	else
	{//Quantum has not expired, so we'll just end the critical section. 
		MutexUnlock( & SchedulerLock );
		InterruptEnable();
	}
}

BOOL SchedulerIsCritical()
{
	return MutexIsLocked( & SchedulerLock );
}

void 
__attribute__((naked,__INTR_ATTRS)) 
SchedulerContextSwitch()
{
	//perfrom context switch
	HAL_SAVE_STATE
	
	HAL_SAVE_SP( ActiveThread->Stack );

	ASSERT( InterruptIsAtomic(), 
			SCHEDULER_CONTEXT_SWITCH_NOT_ATOMIC,
			"Context switch must save state atomically");

	//Check for scheduling event
	if( NextThread != NULL )
	{
		ActiveThread = NextThread;
		NextThread = NULL;
	}

	InterruptEnd(); //reduce interrupt level without enabling interrupts.

	HAL_SET_SP( ActiveThread->Stack );

	HAL_RESTORE_STATE
}

/*Ends a critical section and forces an immediate context switch*/
void  
SchedulerForceSwitch()
{
	ASSERT( MutexIsLocked( & SchedulerLock ),
			SCHEDULER_FORCE_SWITCH_IS_CRITICAL,
			"Schedule will not run when in critical section");

	//This needs to be an atomic operation.
	InterruptDisable();

	//End the critical Section
	//so that we can schedule
	MutexUnlock( & SchedulerLock ); 

	Schedule(); //Schedule next thread manually...

	//Actually context switch.
	SchedulerContextSwitch();
}

void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( MutexIsLocked( & SchedulerLock ), 
			SCHEDULER_RESUME_THREAD_MUST_BE_CRIT,
			"Only run from critical section" );
	ASSERT( thread->State == THREAD_STATE_BLOCKED, 
			SCHEDULER_RESUME_THREAD_NOT_BLOCKED,
			"Thread not blocked" );

	InterruptDisable();
	thread->State = THREAD_STATE_RUNNING;
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) thread, DoneQueue );
	InterruptEnable();
}

void SchedulerBlockThread( )
{
	ASSERT( MutexIsLocked( &SchedulerLock ), 
			SCHEDULER_BLOCK_THREAD_MUST_BE_CRIT,
			"Only block thread from critical section");
	ActiveThread->State = THREAD_STATE_BLOCKED;
}

union BLOCKING_CONTEXT * SchedulerGetBlockingContext( )
{
	ASSERT( MutexIsLocked( & SchedulerLock ),
			SCHEDULER_GET_BLOCKING_CONTEXT_NOT_CRITICAL,
			"Scheduler is not in critical section, cant use blocking context");
	ASSERT( ActiveThread->State == THREAD_STATE_BLOCKED,
			SCHEDULER_GET_BLOCKING_CONTEXT_NOT_BLOCKED,
			"The active thread must be blocked inorder to use the blocking context");

	return & ActiveThread->BlockingContext;
}

void Schedule( void *arg )
{
	//See if we are allowed to schedule (not in crit section)
	if( MutexLock( & SchedulerLock ) )
	{//We are allowed to schedule.
		//save old thread
		if( ActiveThread != &IdleThread && 
				ActiveThread->State == THREAD_STATE_RUNNING)
		{
			LinkedListEnqueue( (struct LINKED_LIST_LINK *) ActiveThread,
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
			ASSERT( NextThread == NULL,
					SCHEDULER_SCHEDULE_NEXT_THREAD_NOT_NULL,
					"Scheduler is running even though NextThread\
					Is not null, this causes thread dropping");
			NextThread = 
				(struct THREAD * ) LinkedListPop( RunQueue );
		}
		else
		{//there were no threads at all, use idle loop.
			NextThread = &IdleThread;
		}

		//restart the scheduler timer if its turned off.
		if( ! SchedulerTimer.Enabled )
		{
			TimerRegister( &SchedulerTimer,
					NextThread->Priority,
					Schedule,
					NULL);
			QuantumExpired = FALSE;
		}

		MutexUnlock( &SchedulerLock );
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
	//Set up Schedule Resource
	MutexLockInit( & SchedulerLock );
	//Create a thread for idle loop.
	SchedulerCreateThread( &IdleThread, 1, NULL, NULL, NULL );
	//Remove IdleThread from queues...
	LinkedListInit( & Queue1 );
	LinkedListInit( & Queue2 );
	//Initialize ActiveThread
	ActiveThread = & IdleThread;
	NextThread = NULL;
}

struct THREAD * SchedulerGetActiveThread()
{
	return ActiveThread;
}

void SchedulerCreateThread( 
		struct THREAD * thread,
		unsigned char priority,
		char * stack,
		unsigned int stackSize,
		THREAD_MAIN main)
{
	//Populate thread struct
	thread->Priority = priority;
	thread->State = THREAD_STATE_RUNNING;
	//initialize stack
	thread->Stack = (char*)((unsigned int) stack + stackSize);
	HalCreateStackFrame( thread, main );
	//Add thread to done queue.
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) thread, DoneQueue );
}	
