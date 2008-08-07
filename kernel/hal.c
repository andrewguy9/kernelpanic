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

//Assembly to aid in context switch

#define HAL_SAVE_STATE \
		asm( \
			"push r0\n\t" \
			"push r1\n\t" \
			"push r2\n\t" \
			"push r3\n\t" \
			"push r4\n\t" \
			"push r5\n\t" \
			"push r6\n\t" \
			"push r7\n\t" \
			"push r8\n\t" \
			"push r9\n\t" \
			"push r10\n\t" \
			"push r11\n\t" \
			"push r12\n\t" \
			"push r13\n\t" \
			"push r14\n\t" \
			"push r15\n\t" \
			"push r16\n\t" \
			"push r17\n\t" \
			"push r18\n\t" \
			"push r19\n\t" \
			"push r20\n\t" \
			"push r21\n\t" \
			"push r22\n\t" \
			"push r23\n\t" \
			"push r24\n\t" \
			"push r25\n\t" \
			"push r26\n\t" \
			"push r27\n\t" \
			"push r28\n\t" \
			"push r29\n\t" \
			"push r30\n\t" \
			"push r31\n\t" \
			"in r0, 0x3F\n\t" \
			"push r0\n\t" \
			);

#define HAL_RESTORE_STATE \
	asm( \
            "pop r0\n\t" \
            "out 0x3F, r0\n\t" \
            "pop r31\n\t" \
            "pop r30\n\t" \
            "pop r29\n\t" \
            "pop r28\n\t" \
            "pop r27\n\t" \
            "pop r26\n\t" \
            "pop r25\n\t" \
            "pop r24\n\t" \
            "pop r23\n\t" \
            "pop r22\n\t" \
            "pop r21\n\t" \
            "pop r20\n\t" \
            "pop r19\n\t" \
            "pop r18\n\t" \
            "pop r17\n\t" \
            "pop r16\n\t" \
            "pop r15\n\t" \
            "pop r14\n\t" \
            "pop r13\n\t" \
            "pop r12\n\t" \
            "pop r11\n\t" \
            "pop r10\n\t" \
            "pop r9\n\t" \
            "pop r8\n\t" \
            "pop r7\n\t" \
            "pop r6\n\t" \
            "pop r5\n\t" \
            "pop r4\n\t" \
            "pop r3\n\t" \
            "pop r2\n\t" \
            "pop r1\n\t" \
            "pop r0\n\t" \
			"ret\n\t" \
			);

//Hal Save SP and Set SP are used by the avr
//context switch to manipulate the stack pointer.
#define HAL_SAVE_SP(dest) (dest = (void *) SP)
#define HAL_SET_SP(value) (SP = (int) (value) )

#define BAUD_19200 51
#define BAUD_9600 103
#define BAUD_56000 16

#define UCSZ_8BIT 3<<1

//Defines for the timer.
#define TMR_PRESCALE_NONE  0xF4
#define TMR_PRESCALE_1024  0x07
#define TMR_PRESCALE_MASK  0x07
#define TMR_MS             16


#define DEBUG_SW           PINA
#define DEBUG_SW_PORT      PORTA
#define DEBUG_SW_DDR       DDRA

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

void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize)
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
	//Stack complete, place in machine context.
	Context->Stack = stack;

	//Save the stack size boundaries.
#ifdef DEBUG
		Context->High = stack + stackSize;
		Context->Low = stack;
#endif
}

void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context )
{
	//Null out inital stack pointer since it will be set on first
	//context switch.
	Context->Stack = NULL;

	//The stack bounderies are infinite for the initial stack.
#ifdef DEBUG
		Context->High = (char *) -1;
		Context->Low = (char *) 0;
#endif
}

void HalSerialStartup()
{
	//turn on the serial system.
	
	UBRR = BAUD_19200;
	UCSRC = UCSZ_8BIT;
	UCSRB = _BV(TXCIE) | _BV(RXCIE) | _BV(RXEN) | _BV(TXEN);
}

void HalContextSwitch(struct MACHINE_CONTEXT * oldContext, struct MACHINE_CONTEXT * newContext )
{
	//perfrom context switch
	HAL_SAVE_STATE
	
	HAL_SAVE_SP( oldContext->Stack );

	//Check to see if stack has overflowed.
#ifdef DEBUG
	ASSERT( ASSENDING( 
				(unsigned int) oldContext->Low, 
				(unsigned int) oldContext->Stack, 
				(unsigned int) oldContext->High ) );
#endif

	HAL_SET_SP( newContext->Stack );

	HAL_RESTORE_STATE
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

#define SAVE_STATE( context ) \
	(void)getcontext( &(context)->State)

#define RESTORE_STATE( context ) \
	(void)setcontext(&(context)->State)

#define SWITCH_CONTEXT( old, new ) \
	(void)swapcontext(&((old)->State), &((new)->State))


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

void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize)
{
	getcontext(&(Context->State));

	/* adjust to new context */
	Context->State.uc_link = NULL;
	Context->State.uc_stack.ss_sp = stack;
	Context->State.uc_stack.ss_size = stackSize;
	Context->State.uc_stack.ss_flags = 0;

	/*make new context */
	makecontext( &(Context->State), foo, 1, NULL );

	//Save the stack size boundaries.
#ifdef DEBUG
		Context->High = stack + stackSize;
		Context->Low = stack;
#endif
}

void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context )
{
	//Store the system's stste
	getcontext(&(Context->State));
	//The stack bounderies are infinite for the initial stack.
#ifdef DEBUG
		Context->High = (char *) -1;
		Context->Low = (char *) 0;
#endif
}

void HalContextSwitch(struct MACHINE_CONTEXT * oldContext, struct MACHINE_CONTEXT * newContext )
{
	(void)swapcontext(&((oldContext)->State), &((newContext)->State));
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

	signal(SIGVTALRM, HalLinuxTimer);
}
#endif //end of pc build
//-----------------------------------------------------------------------------

