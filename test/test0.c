#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#define DEBUG_LED          PORTC
#define DEBUG_LED_DDR      DDRC

//Tests of the Timer subsystem.
struct HANDLER_OBJECT FrequentTimer;
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

struct HANDLER_OBJECT SeldomTimer;
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
