#include "hal.h"

/*
 * HAL UNIT DESCRIPTION
 *
 * The code for each hardware type should be surrounded with #ifdef ... #endif.
 *
 * All hardware dependant code should be placed in the hal unit.
 * Much of the hardware dependant code is impemented through macros.
 *
 * Functions prototypes should be hardware generic, placed once in the header and 
 * implemented for each hardware type in the source file.
 *
 * Macros should be implemented in the header for each supported hardware type.
 */

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

void * HalCreateStackFrame( void * stack, THREAD_MAIN main, COUNT stackSize )
{	
	//create initial stack frame
	stack = (char*)((unsigned int) stack + stackSize);//Pick which end of stack
	stack -= sizeof( void * );
	//Drop in main frame
	*((unsigned char*)stack + 1) = (int) main;
    *((unsigned char *)(stack)) = 
		(unsigned char)((unsigned int)(main)>>8);
	//Add context restore frame
	stack -= 34*sizeof(char);
	//Stack complete
	return stack;
}

#include <avr/io.h>
//usart registers
#define UCSRA   UCSR0A
#define UCSRB   UCSR0B
#define UCSRC   UCSR0C
#define UDR     UDR0
#define UBRR    UBRR0L

#define BAUD_19200 51
#define BAUD_9600 103
#define BAUD_56000 16

#define UCSZ_8BIT 3<<1

void HalSerialStartup()
{
	UBRR = BAUD_19200;
	UCSRC = UCSZ_8BIT;
	UCSRB = _BV(TXCIE) | _BV(RXCIE) | _BV(RXEN) | _BV(TXEN);
}
#endif
//-----------------------------------------------------------------------------

