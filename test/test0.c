#include"kernel/startup.h"
#include"kernel/timer.h"

//
//Tests of the Timer subsystem.
//FrequentTimer should be run more often than SeldomTimer.
//So FequentCount should be larger than SeldomHandler.
//Time should be greather than both.
//

COUNT FrequentCount;
COUNT SeldomCount;

struct HANDLER_OBJECT FrequentTimer;
HANDLER_FUNCTION FrequentHandler;
BOOL FrequentHandler( struct HANDLER_OBJECT * timer )
{
	int * count = timer->Context;

	ASSERT( timer == &FrequentTimer );

	(*count)++;

	TimerRegister( 
			timer,
			20,
			FrequentHandler,
		    count );
	return FALSE;
}

struct HANDLER_OBJECT SeldomTimer;
HANDLER_FUNCTION SeldomHandler;
BOOL SeldomHandler( struct HANDLER_OBJECT * timer )
{
	int * count = timer->Context;

	ASSERT( timer == &SeldomTimer );

	(*count)++;

	TimerRegister(
			&SeldomTimer,
			40,
			SeldomHandler,
			count );
	return FALSE;
}

int main()
{
	//Initialize the kernel structures.
	KernelInit();

	//Initialize timers.
	FrequentCount = 0;
	HandlerInit(&FrequentTimer);
	TimerRegister( 
			&FrequentTimer,
			20,
			FrequentHandler,
		    &FrequentCount );


	SeldomCount = 0;
	HandlerInit(&SeldomTimer);
	TimerRegister( 
			&SeldomTimer,
			40,
			SeldomHandler,
			&SeldomCount );

	KernelStart();
	return 0;
}
