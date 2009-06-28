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
int Deviation;

void ValidateState()
{
	ASSERT( Deviation <= 3 && Deviation >= -3 );
}

struct POST_HANDLER_OBJECT FrequentTimer;
void FrequentHandler( void * arg )
{
	struct POST_HANDLER_OBJECT * timer = arg;

	int * count = timer->Context;

	ASSERT( timer == &FrequentTimer );

	(*count)++;
	Deviation++;

	ValidateState();

	TimerRegister( 
			timer,
			2,
			FrequentHandler,
		    count );
}

struct POST_HANDLER_OBJECT SeldomTimer;
void SeldomHandler( void * arg )
{
	struct POST_HANDLER_OBJECT * timer = arg;

	int * count = timer->Context;

	ASSERT( timer == &SeldomTimer );

	(*count)++;
	Deviation-=2;

	ValidateState();

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

	Deviation = 0;
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
