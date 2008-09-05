#include"../kernel/startup.h"
#include"../kernel/timer.h"

//
//Tests of the Timer subsystem.
//FrequentTimer should be run more often than SeldomTimer.
//So FequentCount should be larger than SeldomHandler.
//Time should be greather than both.
//

struct POST_HANDLER_OBJECT FrequentTimer;
COUNT FrequentCount;
void FrequentHandler( void * arg )
{
	struct POST_HANDLER_OBJECT * timer = arg;

	int * count = timer->Context;

	ASSERT( timer == &FrequentTimer );

	(*count)++;

	TimerRegister( 
			timer,
			2,
			FrequentHandler,
		    count );
}

struct POST_HANDLER_OBJECT SeldomTimer;
COUNT SeldomCount;
void SeldomHandler( void * arg )
{
	SeldomCount++;
	TimerRegister(
			&SeldomTimer,
			3,
			SeldomHandler,
		   NULL	);
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
			3,
			SeldomHandler,
			&SeldomCount );

	KernelStart();
	return 0;
}
