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
 *
 * Startup routines should be only called witih interrupts disabled,
 * so they dont have to gaurantee atomicy. 
 */

//
//AVR CODE
//
#ifdef AVR_BUILD

void HalStartup()
{
	//Turn setup generic registers.
	//Not timing or interrupt registers.
	DEBUG_LED_DDR = 0xff;
    DEBUG_SW_DDR = 0x00;
    DEBUG_SW_PORT = 0xff;
}

void HalInitClock()
{
	//Initialize the clock system.
	
	//preload the timer counter
	TCNT0 = 0xff-1*TMR_MS; //1 ms

	//enable interrupts
	TIMSK |= _BV(TOIE0);

	//set clock source to start timer
	TCCR0 |= TMR_PRESCALE_1024;
}

void * HalCreateStackFrame( void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize )
{	
	//create initial stack frame
	stack = (char*)((unsigned int) stack + stackSize);//Pick which end of stack
	stack -= sizeof( void * );
	//Drop in the kickoff routine
	*((unsigned char*)stack + 1) = (int) foo;
    *((unsigned char *)(stack)) = 
		(unsigned char)((unsigned int)(foo)>>8);
	//Add context restore frame
	stack -= 34*sizeof(char);
	//Stack complete
	return stack;
}

void HalSerialStartup()
{
	//turn on the serial system.
	
	UBRR = BAUD_19200;
	UCSRC = UCSZ_8BIT;
	UCSRB = _BV(TXCIE) | _BV(RXCIE) | _BV(RXEN) | _BV(TXEN);
}
#endif// end avr build
//-----------------------------------------------------------------------------
//
//PC CODE
//
#ifdef PC_BUILD

#include<sys/time.h>
#include<signal.h>

char DEBUG_LED;
BOOL AtomicState;
struct itimerval TimerInterval;

void HalLinuxTimer();

void HalStartup()
{
	DEBUG_LED = 0;
	AtomicState = TRUE;
}

void HalInitClock()
{
	int result;
	
	//Set the timer handler.
	signal(SIGVTALRM, HalLinuxTimer );

	//Set the timer interval.
	TimerInterval.it_interval.tv_sec = 0;
	TimerInterval.it_interval.tv_usec = 1;
	TimerInterval.it_value.tv_sec = 0;
	TimerInterval.it_value.tv_usec = 1;
	result = setitimer( ITIMER_VIRTUAL, &TimerInterval, NULL );
	ASSERT(result == 0 );
}

void * HalCreateStackFrame( void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize )
{
	//TODO	
	return NULL;
}

void HalSerialStartup()
{
	//TODO
}

inline BOOL HalIsAtomic()
{
	return AtomicState;
}

inline void HalDisableInterrupts()
{
	AtomicState = TRUE;
}

inline void HalEnableInterrupts()
{
	AtomicState = FALSE;
}


//prototype for handler.
void TimerInterrupt();

/*
 * Acts like the hardware clock.
 * Calls TimerInterrupt if he can.
 */
void HalLinuxTimer()
{
	//Check to see if we are atomic. If we are
	//then the timer "could not have fired" and we
	//can return. Otherwise call timer.
	
	signal(SIGVTALRM, HalLinuxTimer);//remember to reset the timer...

	if( AtomicState )
		return;
	else//AtomicState == 0
	{
		AtomicState = TRUE;//We are simulating a hardware interrupt, so atomic goes to true.
		TimerInterrupt();
		AtomicState = FALSE;//We are simulating a hardware interrupt, so when timer interrupt returns there is an implied release of interrupt flag.
	}
}

#endif //end of pc build
//-----------------------------------------------------------------------------

