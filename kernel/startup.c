#include"timer.h"
#include"scheduler.h"
#include"hal.h"

void main()
{//entry point for kernel...
	HalInit();

	TimerInit( );

	SchedulerInit();

	HalEnableInterrupts();

}
