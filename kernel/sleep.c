#include"sleep.h"
#include"scheduler.h"

#include"hal.h"

void SleepHandler(void * arg)
{
	struct THREAD * thread = (struct THREAD *) arg;
	SchedulerResumeThread( thread );
}

void Sleep( COUNT time )
{
	//start critical section
	SchedulerStartCritical();
	//block thread ( no context now )
	SchedulerBlockThread( 0 );
	//initialize timer to wake thread.
	HalDisableInterrupts();
	struct THREAD * thread = SchedulerGetActiveThread();
	struct TIMER * timer = & thread->BlockingContext.SleepTimer;
	TimerRegister( timer, time, SleepHandler, thread );

	//re-enable interrupts
	//NOTE:we can re-enable interrupts, the scheduler is still locked
	//so it wont attempt to schedule this thread before we are ready.
	HalEnableInterrupts();

	//go ahead and stop the thread.
	SchedulerForceSwitch();
}
