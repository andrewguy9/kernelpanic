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
	FrequentCount++;
	TimerRegister( 
			&FrequentTimer,
			2,
			FrequentHandler,
		   NULL	);
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
	TimerRegister( 
			&FrequentTimer,
			2,
			FrequentHandler,
		   NULL	);

	SeldomCount = 0;
	TimerRegister( 
			&SeldomTimer,
			3,
			SeldomHandler,
		   NULL	);

	KernelStart();
	return 0;
}
