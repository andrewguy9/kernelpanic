#include "hal.h"

/*
 * HAL UNIT DESCRIPTION
 *
 * The hal is responsible for all hardware dependant code.
 *
 * Rules:
 * The code for each hardware type should be surrounded with #ifdef ... #endif.
 *
 * All hardware dependant code should be placed in the hal unit.
 * Much of the hardware dependant code is impemented through macros.
 *
 * Functions prototypes should be hardware generic, placed once in the header 
 * and implemented for each hardware type in the source file.
 *
 * Macros should be implemented in the header for each supported hardware type.
 *
 * Startup routines should be only called witih interrupts disabled,
 * so they dont have to gaurantee atomicy. 
 */

extern struct MACHINE_CONTEXT * ActiveStack;
extern struct MACHINE_CONTEXT * NextStack;

//
//AVR CODE
//

#ifdef AVR_BUILD

#include<avr/wdt.h>

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
	//Setup debug led port IO Direction
	HAL_WATCHDOG_MASK_DDR = 0xff;
	//Setup Switch IO Direction
    DEBUG_SW_DDR = 0x00;
	//set Switch IO values
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
	char reg;
	char * sp = (char *) stack;

	//create initial stack frame
	sp = (char*)((unsigned int) stack + stackSize);//Pick which end of stack
	sp -= sizeof( void * );
	
	//Drop in the kickoff routine (function pointer)
	*((unsigned char*)sp + 1) = (int) foo;
    *((unsigned char *)(sp)) = 
		(unsigned char)((unsigned int)(foo)>>8);

	//Add context restore frame (regs r0-r31 + sreg)
	for( reg = 0; reg < 33; reg++ )
	{
		sp -= sizeof(char);
		*sp = reg;
	}

	//overwrite last register (sreg) with 0 (interrupt disabled, all flags 0 )
	*sp = 0x00;
	sp -= sizeof(char);

	//Stack complete, place in machine context.
	Context->Stack = sp;

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

void HalContextSwitch( )
{
	//perfrom context switch
	HAL_SAVE_STATE
	
	HAL_SAVE_SP( ActiveStack->Stack );

	//Check to see if stack has overflowed.
#ifdef DEBUG
	ASSERT( ASSENDING( 
				(unsigned int) ActiveStack->Low, 
				(unsigned int) ActiveStack->Stack, 
				(unsigned int) ActiveStack->High ) );
#endif

	//Switch the stacks.
	ActiveStack = NextStack;
	NextStack = NULL;

	HAL_SET_SP( ActiveStack->Stack );

	HAL_RESTORE_STATE
}

void HalPanic(char file[], int line)
{
	return;
}

void HalStartupWatchdog( int frequency )
{
	if( frequency <= 15 )
	{
		wdt_enable( WDTO_15MS );
	} 
	else if( frequency <= 30 )
	{
		wdt_enable( WDTO_30MS );
	}
	else if( frequency <= 60 )
	{
		wdt_enable( WDTO_60MS );
	}else if( frequency <= 120 )
	{
		wdt_enable( WDTO_120MS );
	}
	else if( frequency <= 250 )
	{
		wdt_enable( WDTO_250MS );
	}
	else if( frequency <= 500 )
	{
		wdt_enable( WDTO_500MS );
	}
	else if( frequency <= 1000 )
	{
		wdt_enable( WDTO_1S );
	}
	else 
	{
		wdt_enable( WDTO_2S );
	}
}

void HalPetWatchdog( )
{
	wdt_reset();
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
#include<stdlib.h>
#include<stdio.h>

#define SAVE_STATE( context ) \
	(void)getcontext( &(context)->uc)

#define RESTORE_STATE( context ) \
	(void)setcontext(&(context)->uc)

#define SWITCH_CONTEXT( old, new ) \
	(void)swapcontext(&((old)->uc), &((new)->uc))

#define SET_SIGNAL(signum, handler) \
	signal(signum, handler )

#define SIG_PROC_MASK( new, old ) \
	sigprocmask( SIG_SETMASK, new, old )

#define AlarmSignal SIGVTALRM
#define InterruptFlagSignal SIGUSR1

char HAL_WATCHDOG_MASK;
BOOL HalWatchdogOn;
unsigned int HalWatchDogFrequency;
unsigned int HalWatchdogCount;

sigset_t InterruptDisabledSet;//Set of interrupts which are disabled while atomic
sigset_t InterruptEnabledSet;//set of interrupt which are disabled while in thread

struct itimerval TimerInterval;

volatile BOOL atomic;

//Prototype for later use.
void HalLinuxTimer();

void HalStartup()
{
	atomic = TRUE;
	HalWatchdogOn = FALSE;
	HAL_WATCHDOG_MASK = 0;
	HalWatchDogFrequency = 0;
	HalWatchdogCount = 0;
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
	SET_SIGNAL( AlarmSignal, HalLinuxTimer );
}

void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize)
{
	SAVE_STATE(Context);

	/* adjust to new context */
	Context->uc.uc_link = NULL;
	Context->uc.uc_stack.ss_sp = stack;
	Context->uc.uc_stack.ss_size = stackSize;
	Context->uc.uc_stack.ss_flags = 0;

	/*make new context */
	makecontext( &(Context->uc), foo, 0 );

	//Save the stack size boundaries.
#ifdef DEBUG
		Context->High = stack + stackSize;
		Context->Low = stack;
#endif
}

void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context )
{
	//Store the system's stste
	SAVE_STATE(Context);
	//The stack bounderies are infinite for the initial stack.
#ifdef DEBUG
		Context->High = (char *) -1;
		Context->Low = (char *) 0;
#endif
}

void HalContextSwitch( )
{
	struct MACHINE_CONTEXT * oldContext = ActiveStack;
	struct MACHINE_CONTEXT * newContext = NextStack;

	ActiveStack = NextStack;
	NextStack = NULL;

	SWITCH_CONTEXT(oldContext, newContext);
}

void HalSerialStartup()
{
	//TODO
}

BOOL HalIsAtomic()
{
	return atomic;
}

void HalDisableInterrupts()
{
	atomic = TRUE;
}

void HalEnableInterrupts()
{
	atomic = FALSE;
}

//prototype for handler.
void TimerInterrupt();

/*
 * Acts like the hardware clock.
 * Calls TimerInterrupt if he can.
 */
void HalLinuxTimer()
{
	//Do watchdog check irrespective of weather we are atomic.
	if( HalWatchdogOn )
	{
		HalWatchdogCount ++;

		if( HalWatchdogCount >= HalWatchDogFrequency )
		{
			//The time for a match has expried. Panic!!!
			printf("WATCHDOG TIMEOUT!!!");
			exit(0);
		}
	}

	if( ! atomic )
	{

		//There is an implied disable interrupts call when the timer fires. 
		HalDisableInterrupts();

		//Call the kernel's timer handler.
		TimerInterrupt();

		//There is an implied enable interrupts call when the timer
		//returns.
		HalEnableInterrupts();

	}

}

void HalResetClock()
{
}

void HalPanic(char file[], int line)
{
	printf("PANIC: %s:%d\n",file,line);
	exit(-1);
}

void HalStartupWatchdog( int frequency )
{
	HalWatchdogOn = TRUE;
	HalWatchDogFrequency = frequency;
}

void HalPetWatchdog( )
{
	HalWatchdogCount = 0;	
}

#endif //end of pc build
//-----------------------------------------------------------------------------
