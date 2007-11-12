#include"sleep.h"
#include"scheduler.h"

#include"interrupt.h"

/*
 * Sleep Unit Description
 * Allows threads to request a time out from execution. 
 * A timer is added to the timer heap which will wake the 
 * thread when it expires. 
 */

void SleepHandler(void * arg)
{
	struct THREAD * thread = (struct THREAD *) arg;

	ASSERT( thread->State == THREAD_STATE_BLOCKED,
			SLEEP_HANDLER_THREAD_AWAKE,
			"Cannot wake a thread that is running.");

	SchedulerResumeThread( thread );
}

void Sleep( COUNT time )
{
	//start critical section
	SchedulerStartCritical();
	//block thread
	SchedulerBlockThread();
	//Get data to build timer.
	union BLOCKING_CONTEXT * context = SchedulerGetBlockingContext();
	struct THREAD * thread = SchedulerGetActiveThread();
	//build timer into blocking context
	TimerRegister( &context->SleepTimer,
			time,
			SleepHandler,
			thread );

	//go ahead and stop the thread.
	SchedulerForceSwitch();
}
