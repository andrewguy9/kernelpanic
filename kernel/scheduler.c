#include"scheduler.h"
#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"timer.h"
#include"mutex.h"
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

struct THREAD * ActiveThread;
struct THREAD * NextThread;

//Used to mark critical sections...
struct MUTEX SchedulerLock;

//Scheduler variables: Only edit atomically.
//This is because the scheduler is an interrupt
//level entitiy.
struct LINKED_LIST Queue1;
struct LINKED_LIST Queue2;
struct LINKED_LIST * RunQueue;
struct LINKED_LIST * DoneQueue;

struct TIMER SchedulerTimer;

//Variables that need to be atomic.
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
	ASSERT( MutexIsLocked( & SchedulerLock ),
			SCHEDULER_END_CRITICAL_NOT_CRITICAL,
		   	"Critical section cannot start.");

	//We may have to fire the scheduler
	//so we need to be atomic.
	HalDisableInterrupts();

	//End the critical section
	MutexUnlock( & SchedulerLock );

	if( QuantumExpired )
	{//Quantum has expired while in crit section, fire manually.
		SchedulerForceSwitch();
	}
	else
	{//Quantum has not expired, so we'll just end the critical section. 
		HalEnableInterrupts();
	}
}

/*Ends a critical section and forces an immediate context switch*/
void SchedulerForceSwitch()
{
	//TODO this function may be broken: 
	//verify that InterruptLevel is 0, 
	//and that reti clears the interrupt bit.
	//Having the stack line up would be nice too.

	//ASSERT( ! HalIsAtomic(),
	//		SCHEDULER_FORCE_SWITCH_IS_ATOMIC,
	//		"Schedule Force Switch starts its own atomic \
	//		section, so interrupts must be off.");
	ASSERT( MutexIsLocked( & SchedulerLock ),
			SCHEDULER_FORCE_SWITCH_IS_CRITICAL,
			"Schedule will not run when in critical section");

	HalDisableInterrupts();

	MutexUnlock( & SchedulerLock ); //End the critical Section.

	Schedule(); //Schedule next thread manually...

	HalEndInterrupt(); 

	TimerInterrupt();
}

void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( MutexIsLocked( & SchedulerLock ), 
			SCHEDULER_RESUME_THREAD_MUST_BE_CRIT,
			"Only run from critical section" );
	ASSERT( thread->State == THREAD_STATE_BLOCKED, 
			SCHEDULER_RESUME_THREAD_NOT_BLOCKED,
			"Thread not blocked" );

	HalDisableInterrupts();
	thread->State = THREAD_STATE_RUNNING;
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) thread, DoneQueue );
	HalEnableInterrupts();
}

void SchedulerBlockThread( )
{
	ASSERT( MutexIsLocked( &SchedulerLock ), 
			SCHEDULER_BLOCK_THREAD_MUST_BE_CRIT,
			"Only block thread from critical section");
	//We only look at this value when we are actually running the scheduler!
	//which can never happen when in a critical section.
	//So we dont have to be at interrupt level.
	ActiveThread->State = THREAD_STATE_BLOCKED;

}

void Schedule( )
{
	ASSERT( HalIsAtomic(), 
			SCHEDULE_MUST_BE_ATOMIC,
			"Only run schedule in interrupt mode");

	//See if we are allowed to schedule (not in crit section)
	if( ! MutexIsLocked( & SchedulerLock ) )
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
					Schedule);
			QuantumExpired = FALSE;
		}
	}
	else
	{//we are not allowed to schedule.
		//mark the quantum as expired.
		QuantumExpired = TRUE;
	}
}//end Schedule

void SchedulerInit()
{
	//Initialize queues
	LinkedListInit( & Queue1 );
	LinkedListInit( & Queue2 );
	RunQueue = & Queue1;
	DoneQueue = & Queue2;
	//Initialize the timer
	TimerRegister( & SchedulerTimer,
		   	0, 
			Schedule );
	QuantumExpired = FALSE;
	//Set up Schedule Resource
	MutexLockInit( & SchedulerLock );
	//Create a thread for idle loop.
	SchedulerCreateThread( &IdleThread, 1, NULL, NULL, NULL );
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
