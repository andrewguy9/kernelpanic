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
#include<string.h>
#include<signal.h>

char DEBUG_LED;

sigset_t InterruptDisabledSet;
sigset_t InterruptEnabledSet;

struct itimerval TimerInterval;

//Prototype for later use.
void HalLinuxTimer();

void HalStartup()
{
	int ret;
	DEBUG_LED = 0;

	//
	//Set up signal masks for atomic and non atomic sections.
	//
	
	//Allow all signals.
	ret = sigemptyset( &InterruptDisabledSet );
	ASSERT( ret == 0 );
	ret = sigemptyset( &InterruptEnabledSet );
	ASSERT( ret == 0 );

	//When interrupts are disabled, turn off the clock and user1.
	ret = sigaddset( &InterruptDisabledSet, SIGVTALRM );
	ASSERT( ret == 0 );
	ret = sigaddset( &InterruptDisabledSet, SIGUSR1 );
	ASSERT( ret == 0 );

	//Turn off signal handlers since hardware starts in disabled state.
	ret = sigprocmask( SIG_SETMASK, &InterruptDisabledSet, NULL );
	ASSERT( ret == 0 );
}

void HalInitClock()
{
	int result;
	
	//Set the timer interval.
	TimerInterval.it_interval.tv_sec = 0;
	TimerInterval.it_interval.tv_usec = 1;
	TimerInterval.it_value.tv_sec = 0;
	TimerInterval.it_value.tv_usec = 1;
	result = setitimer( ITIMER_VIRTUAL, &TimerInterval, NULL );
	ASSERT(result == 0 );

	//Turn on the timer signal handler.
	signal( SIGVTALRM, HalLinuxTimer );
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

BOOL HalIsAtomic()
{
	int ret;
	sigset_t curState;
	
	//Get the current signal mask
	ret = sigprocmask( 0, NULL, &curState );
	ASSERT( ret == 0 );

	//See if the SIGUSR1 signal is enabled (since it is used to simulate the interrupt flag.)
	ret = sigismember( &curState, SIGUSR1 );

	if( ret == 1 )
	{
		//SIGUSR1 is masked, so interrupts must be disabled.
		return TRUE;
	}
	else if( ret == 0 )
	{
		//SIGUSR1 is not masked, so interrupts must be enabled.
		return FALSE;
	}
	else 
	{
		ASSERT(0);
		return TRUE;
	}
}

void HalDisableInterrupts()
{
	int ret;
	ret = sigprocmask(SIG_SETMASK, &InterruptDisabledSet, NULL );
	ASSERT( ret == 0 );
}

void HalEnableInterrupts()
{
	int ret;
	ret = sigprocmask(SIG_SETMASK, &InterruptEnabledSet, NULL );
	ASSERT( ret == 0 );
}

//prototype for handler.
void TimerInterrupt();

/*
 * Acts like the hardware clock.
 * Calls TimerInterrupt if he can.
 */
void HalLinuxTimer()
{
	//There is an implied disable interrupts call when the timer fires. 
	HalDisableInterrupts();

	//Call the kernel's timer handler.
	TimerInterrupt();

	//There is an implied enable interrupts call when the timer
	//returns.
	HalEnableInterrupts();
}

void HalResetClock()
{
	int ret;

	ret = signal(SIGVTALRM, HalLinuxTimer);
	ASSERT( ret == 0 );
}
#endif
//-----------------------------------------------------------------------------

