#include"../kernel/startup.h"
#include"../kernel/timer.h"

//
//Tests of the Timer subsystem.
//FrequentTimer should be run more often than SeldomTimer.
//So FequentCount should be larger than SeldomHandler.
//Time should be greather than both.
//

COUNT FrequentCount;
COUNT SeldomCount;

struct HANDLER_OBJECT FrequentTimer;
void FrequentHandler( struct HANDLER_OBJECT * timer )
{
	int * count = timer->Context;

	ASSERT( timer == &FrequentTimer );

	(*count)++;

	TimerRegister( 
			timer,
			2,
			FrequentHandler,
		    count );
}

struct HANDLER_OBJECT SeldomTimer;
void SeldomHandler( struct HANDLER_OBJECT * timer )
{
	int * count = timer->Context;

	ASSERT( timer == &SeldomTimer );

	(*count)++;

	TimerRegister(
			&SeldomTimer,
			4,
			SeldomHandler,
			count );
}

int main()
{
	//Initialize the kernel structures.
	KernelInit();

	//Initialize timers.
	FrequentCount = 0;
	TimerInit(&FrequentTimer);
	TimerRegister( 
			&FrequentTimer,
			2,
			FrequentHandler,
		    &FrequentCount );


	SeldomCount = 0;
	TimerInit(&SeldomTimer);
	TimerRegister( 
			&SeldomTimer,
			4,
			SeldomHandler,
			&SeldomCount );

	KernelStart();
	return 0;
}
