#include"sleep.h"
#include"scheduler.h"
#include"timer.h"
#include"interrupt.h"

/*
 * Sleep Unit Description
 * Allows threads to request a time out from execution. 
 */

void Sleep( COUNT time )
{
	TIME wakeTime = TimerGetTime()+time;
	while( wakeTime > TimerGetTime() )
	{
		SchedulerStartCritical();
		SchedulerForceSwitch();
	}
	return;
}
