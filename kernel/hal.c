#include "hal.h"

//
//HARDWARE INDEPENDANT CODE
//
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
	DEBUG_LED_DDR = 0xff;
    DEBUG_SW_DDR = 0x00;
    DEBUG_SW_PORT = 0xff;

	InterruptLevel = 1;
}

//
//AVR CODE
//
#ifdef AVR_BUILD
void HalInitClock()
{
	//preload the timer counter
	TCNT0 = 0xff-1*TMR_MS; //1 ms

	//enable interrupts
	TIMSK |= _BV(TOIE0);

	//set clock source to start timer
	TCCR0 |= TMR_PRESCALE_1024;
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
#endif
//-----------------------------------------------------------------------------





