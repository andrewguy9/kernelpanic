#include"sleep.h"
#include"scheduler.h"
#include"timer.h"

/*
 * Sleep Unit Description
 * Allows threads to request a time out from execution. 
 */

//TODO NOW THAT WE HAVE WORK ITEMS IN KERNEL, YOU SHOULD TRY TO MOVE AWAY FROM POLLING FOR SLEEPING.
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
