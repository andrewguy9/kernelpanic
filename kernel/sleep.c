#include"sleep.h"
#include"scheduler.h"
#include"timer.h"
#include"critinterrupt.h"

/*
 * This function is called by the SleepTimerHandler when
 * its time to wake a thread. This function runs in 
 * a critical section so it can wake a thread.
 */
BOOL SleepCritHandler( struct HANDLER_OBJECT * handler ) 
{
	struct THREAD * thread = handler->Context;

	ASSERT( SchedulerIsCritical() );
	SchedulerResumeThread( thread );

	return TRUE;
}

/*
 * This function is called when a thread has 
 * called Sleep(), and the time he specified 
 * has passed. 
 *
 * We will schedule a critical handler which can wake threads.
 */
BOOL SleepTimerHandler( struct HANDLER_OBJECT * timer )
{
	CritInterruptRegisterHandler(
			timer, 
			SleepCritHandler, 
			timer->Context );

	return FALSE;
}

/*
 * Sleep registers a timer and then go to sleep.
 * The timer will fire at wake time.
 * The timer cannot reactivate the thread because it may fire 
 * durring a thread critical section. So the timer will register
 * a critical section handler.
 *
 * In order to do this Sleep must use a HANDLER_OBJECT for
 * the timer and crit handler. I will allocate this on the stack.
 */
void Sleep( COUNT time )
{
	struct HANDLER_OBJECT timer;
	struct THREAD * thread;

	//The handler will have to know which thread to wake.
	thread = SchedulerGetActiveThread();

	//We have to enter a critical section because if the timer
	//fires immediatly, we cannot let the worker try to wake the 
	//thread before it has gone to sleep.
	SchedulerStartCritical();

	//Zero out timer.
	HandlerInit( &timer );

	//Register the timer.
	TimerRegister( &timer, time, SleepTimerHandler, thread );

	//Sleep the current thread.
	SchedulerBlockThread();

	//Force the switch.
	SchedulerForceSwitch();

	//If we reach this point, then we have been awakened!
}
