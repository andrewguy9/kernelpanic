#include"sleep.h"
#include"scheduler.h"
#include"timer.h"
#include"worker.h"

/*
 * Sleep Unit Description
 * Allows threads to request a time out from execution. 
 */

struct SLEEP_TIMER_CONTEXT
{
	struct WORKER_ITEM WorkItem;
	struct THREAD * Thread;
};

/*
 * This function is a work item handler which 
 * wakes the thread specified in context.
 */
enum WORKER_RETURN SleepWorkItemHandler( struct WORKER_ITEM * item )
{
	struct THREAD * thread = item->Context;

	SchedulerStartCritical();
	SchedulerResumeThread( thread );
	SchedulerEndCritical();
	
	return WORKER_FINISHED;
}

/*
 * This is function is called when a thread has 
 * called Sleep(), and the time he specified 
 * has passed. 
 *
 * We will schedule a work item to wake the thread.
 */
BOOL SleepTimerHandler( struct HANDLER_OBJECT * timer )
{
	struct SLEEP_TIMER_CONTEXT * sleepContext = timer->Context;

	WorkerInitItem(
			SleepWorkItemHandler,
			sleepContext->Thread,
			& sleepContext->WorkItem);

	return TRUE;
}

/*
 * Sleep registers a timer and then go to sleep.
 * The timer will fire at wake time.
 * The timer cannot reactivate the thread because it may fire 
 * durring a thread critical section. So the timer will register
 * a work item to wake the thread.
 *
 * In order to do this Sleep must use a HANDLER_OBJECT for
 * the timer and a WORKER_ITEM for the work item. We are going
 * to allocate these on the stack.
 */
void Sleep( COUNT time )
{
	struct HANDLER_OBJECT timer;
	struct SLEEP_TIMER_CONTEXT context;

	//The handler will have to know which thread to wake.
	context.Thread = SchedulerGetActiveThread();

	//We have to enter a critical section because if the timer
	//fires immediatly, we cannot let the worker try to wake the 
	//thread before it has gone to sleep.
	SchedulerStartCritical();

	//Zero out timer.
	HandlerInit( &timer );

	//Register the timer.
	TimerRegister( &timer, time, SleepTimerHandler, &context );

	//Sleep the current thread.
	SchedulerBlockThread();

	//Force the switch.
	SchedulerForceSwitch();

	//If we reach this point, then we have been awakened!
}
