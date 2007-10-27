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
	//block thread
	SchedulerBlockThread();
	//Get data to build timer.
	union BLOCKING_CONTEXT * context = SchedulerGetBlockingContext();
	struct THREAD * thread = SchedulerGetActiveThread();
	//build timer into blocking context
	HalDisableInterrupts();
	TimerRegister( &context->SleepTimer,
			time,
			SleepHandler,
			thread );
	HalEnableInterrupts();

	//go ahead and stop the thread.
	SchedulerForceSwitch();
}
