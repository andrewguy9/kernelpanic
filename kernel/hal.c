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

void HalLinuxTimer();

void HalStartup()
{
	DEBUG_LED = 0;

	//Start with all handlers blocked.
	sigemptyset( &InterruptDisabledSet );
	sigemptyset( &InterruptEnabledSet );

	sigaddset( &InterruptEnabledSet, SIGVTALRM );
	sigaddset( &InterruptEnabledSet, SIGUSR1 );
}

void HalInitClock()
{
	int result;
	
	//Set the timer handler.

	//Set the timer interval.
	TimerInterval.it_interval.tv_sec = 0;
	TimerInterval.it_interval.tv_usec = 1;
	TimerInterval.it_value.tv_sec = 0;
	TimerInterval.it_value.tv_usec = 1;
	result = setitimer( ITIMER_VIRTUAL, &TimerInterval, NULL );
	ASSERT(result == 0 );

	HalResetClock();
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

	sigset_t curState;
	
	//Get the current signal mask
	int ret = sigprocmask( SIG_SETMASK, NULL, & curState );
	if( ret != 0 )
	{
		printf("sigprocmask error\n");
		exit(0);
	}

	//See if the SIGUSR1 signal is enabled (since it is used to simulate the interrupt flag.)
	ret = sigismember( &curState, SIGUSR1 );


	if( ret == 1 )
	{
		//SIGUSR1 is enabled, so interrupts must be enabled.
		return FALSE;
	}
	else if( ret == 0 )
	{
		//SIGUSR1 is disabled, so interrupts must be.
		return TRUE;
	}
	else 
	{
		printf("sigismember failed \n");
		exit(0);
	}
}

inline void HalDisableInterrupts()
{
	int ret = sigprocmask( SIG_SETMASK, & InterruptDisabledSet, NULL );
	
	if( ret != 0 )
	{
		printf("sigprocmask error\n");
		exit(0);
	}
}

inline void HalEnableInterrupts()
{
	int ret = sigprocmask( SIG_SETMASK, & InterruptEnabledSet, NULL );

	if( ret != 0 )
	{
		printf("sigprocmask error\n");
		exit(0);
	}
}


//prototype for handler.
void TimerInterrupt();

/*
 * Acts like the hardware clock.
 * Calls TimerInterrupt if he can.
 */
void HalLinuxTimer()
{
	//There is an implied disable interrupts call when the timer
	//fires. 
	HalDisableInterrupts();

	//Call the kernel's timer handler.
	TimerInterrupt();

	//There is an implied enable interrupts call when the timer
	//returns.
	HalEnableInterrupts();
}

inline void HalResetClock()
{
	//signal(SIGVTALRM, HalLinuxTimer);
	struct sigaction action;
	action.sa_handler = HalLinuxTimer;
	action.sa_flags = SA_NODEFER;
	sigemptyset( & action.sa_mask );

	int ret = sigaction(SIGVTALRM, &action, NULL );

	if( ret == -1 )
	{
		printf("sig action failure.\n");
		exit(0);
	}
}
#endif
//-----------------------------------------------------------------------------

