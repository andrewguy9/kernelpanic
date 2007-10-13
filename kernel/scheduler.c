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
 * 2) Provide mechanisms to start and stop
 * threads.
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

struct MUTEX SchedulerLock;
//variables protected by SchedulerLock
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
	ASSERT( aquired, "Start Critical should always stop the scheduler");
}

/*Ends a critical section and forces an immediate context switch*/
void SchedulerForceSwitch()
{

	HalDisableInterrupts();
	HalSaveState();	
	ActiveThread->Stack = (void *) SP;
	Schedule();
	if( NextThread != NULL )
	{
		ActiveThread = NextThread;
		NextThread = NULL;
	}
	SP = (int) ActiveThread->Stack;
	HalRestoreState();
	return;//TODO MAKE SURE THAT STACK STACK IS ALIGNED.
}

void SchedulerEndCritical()
{
	ASSERT( MutexIsLocked( & SchedulerLock ), "Critical section cannot start.");
	MutexUnlock( & SchedulerLock );
	//Check to see if the quantum has expired...
	HalDisableInterrupts();
	if( QuantumExpired )
	{//Quantum has expired while in crit section, fire manually.
		SchedulerForceSwitch();
	}
}

void SchedulerResumeThread( struct THREAD * thread )
{
	ASSERT( MutexIsLocked( & SchedulerLock ), 
			"Only run from critical section" );
	ASSERT( thread->State == THREAD_STATE_BLOCKED, 
			"Thread not blocked" );
	thread->State = THREAD_STATE_RUNNING;
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) thread, DoneQueue );
}

void SchedulerBlockThread( )
{
	ASSERT( MutexIsLocked( &SchedulerLock ), 
			"Only block thread from critical section");
	ActiveThread->State = THREAD_STATE_BLOCKED;
}

void Schedule( ) 
{
	ASSERT( HalIsAtomic(), 
			"Only run schedule in interrupt mode");

	//See if we are allowed to schedule (not in crit section)
	if( MutexIsLocked( & SchedulerLock ) )
	{//We are allowed to schedule.
		//save old thread
		if( ActiveThread->State == THREAD_STATE_RUNNING &&
				ActiveThread != &IdleThread)
		{
			LinkedListEnqueue( (struct LINKED_LIST_LINK *) ActiveThread,
				  DoneQueue);
		}
		//fetch new thread.
		if( LinkedListIsEmpty( RunQueue ) )
		{//there are no threads in first queue.
			struct LINKED_LIST * temp = RunQueue;
			RunQueue = DoneQueue;
			DoneQueue = temp;
		}

		if( LinkedListIsEmpty( RunQueue ) )
		{//there are no threads in 2nd queue, use idle loop
			NextThread = &IdleThread;
		}
		else
		{//there was a thread in 2nd queue, run it
			NextThread = 
				(struct THREAD * ) LinkedListPop( RunQueue );
		}

		//restart the scheduler timer.
		TimerRegisterASR( &SchedulerTimer,
			NextThread->Priority,
			Schedule);
		QuantumExpired = FALSE;
	}
	else
	{
		//mark the quantum as expired.
		QuantumExpired = TRUE;
	}
}

void SchedulerInit()
{
	//Initialize queues
	LinkedListInit( & Queue1 );
	LinkedListInit( & Queue2 );
	RunQueue = & Queue1;
	DoneQueue = & Queue2;
	//Initialize the timer
	TimerRegisterASR( & SchedulerTimer,
		   	0, 
			Schedule );
	QuantumExpired = FALSE;
	//Set up Schedule Resource
	MutexLockInit( & SchedulerLock );
	//Create a thread for idle loop.
	SchedulerCreateThread( &IdleThread, 1, 0, 500, NULL );
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
	//initialize stack
	thread->State = (unsigned int ) stack + stackSize;
	//create initial stack frame
	thread->Stack -= sizeof( void * );
	*(thread->Stack + 1) = (int) main;
    *((unsigned char *)(thread->Stack)) = 
		(unsigned char)((unsigned int)(main)>>8);
	thread->Stack -= 34*sizeof(char);
	//Add thread to done queue.
	LinkedListEnqueue( (struct LINKED_LIST_LINK *) thread, DoneQueue );
}

		
