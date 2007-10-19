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

COUNT InterruptLevel;
void HalDisableInterrupts()
{
	HAL_DISABLE_INTERRUPTS();
	InterruptLevel++;
}

void HalEnableInterrupts()
{
	InterruptLevel--;
	if( InterruptLevel == 0 )
		HAL_ENABLE_INTERRUPTS();
}

void HalStartInterrupt()
{
	InterruptLevel++;
}

void HalEndInterrupt()
{
	InterruptLevel--;
	ASSERT( InterruptLevel == 0,
			HAL_END_INTERRUPT_WRONG_LEVEL,
			"Interrupt level is not balanced at end of interrupt."
		  );
}

void HalInit()
{
	InterruptLevel = 1;
}

void HalCreateStackFrame( struct THREAD * thread, THREAD_MAIN main )
{	
	//create initial stack frame
	thread->Stack -= sizeof( void * );
	*(thread->Stack + 1) = (int) main;
    *((unsigned char *)(thread->Stack)) = 
		(unsigned char)((unsigned int)(main)>>8);
	thread->Stack -= 34*sizeof(char);
}

