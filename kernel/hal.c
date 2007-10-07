#include "hal.h"

void HalInitClock()
{
    /*
	 * TODO VERIFY THAT THE TIMER IS CORRECTLY SET TO 1 MILLISECOND
	 * 8-bit timer should have clk/1024
	 * 15.625 counts is 1ms @ clk/1024
     */

	//preload the timer counter
	TCNT0 = 0xff-1*TMR_MS; //1 ms

	//enable interrupts
	TIMSK |= _BV(TOIE0);

	//set clock source to start timer
	TCCR0 |= TMR_PRESCALE_1024;
}

//-----------------------------------------------------------------------------
BOOL HalIsAtomic()
{
	return SREG&SREG_I;
}

COUNT InterruptLevel;
void HalDisableInterrupts()
{
	asm(" cli");
	InterruptLevel++;
}

void HalEnableInterrupts()
{
	InterruptLevel--;
	if( InterruptLevel == 0 )
		asm(" sei");
}

void HalInit()
{
	InterruptLevel = 1;
}
