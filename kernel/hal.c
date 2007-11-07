#include "hal.h"

//
//HARDWARE INDEPENDANT CODE
//

//
//AVR CODE
//
#ifdef AVR_BUILD

void HalStartup()
{
	DEBUG_LED_DDR = 0xff;
    DEBUG_SW_DDR = 0x00;
    DEBUG_SW_PORT = 0xff;
}

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





