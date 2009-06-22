#ifndef HAL_H
#define HAL_H

//Includes for all builds
#include"../utils/utils.h"
#include"../utils/flags.h"
//-----------------------------------------------------------------------------
//Defines (for all builds)
typedef void (*STACK_INIT_ROUTINE) (void);
//-----------------------------------------------------------------------------
//Prototypes ( For all Builds )
struct MACHINE_CONTEXT;
void HalInitClock();
void HalStartup();
void HalEnableWatchdog( int frequency );
void HalPetWatchdog( );
void HalStartInterrupt();
void HalEndInterrupt();
void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize);
void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context );
void HalSerialStartup();
void HalPanic(char file[], int line);
void HalSleepProcessor();
//-----------------------------------------------------------------------------

//
//AVR DEFINES
//
#ifdef AVR_BUILD

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

/*
 * On avr machines, we manipulate stack pointer directly to
 * implement context switches. So the machine context for an
 * avr machine is the stack pointer itself. The register state
 * is saved directly to the stack, and the top of the stack 
 * is stored in Pointer. Additionally, we protect ourselves
 * from stack overflows by way of the High and Low pointers
 * which point to the Top and Bottom of the stack.
 */

#define HAL_NAKED_FUNCTION __attribute__((naked))
struct MACHINE_CONTEXT
{
	INDEX Flag;//Thread number which gets used for the watchdog and debug leds.
	char * Stack;

#ifdef DEBUG
	//Counters to keep track of thread usage.
	COUNT TimesRun;
	COUNT TimesSwitched;
	//Pointers to the top and buttom of the stack. Used to detect stack overflow.
	char * High;
	char * Low;
#endif
};

//Macros to change machine interrupt state.
#ifdef DEBUG
#define HalIsAtomic() ( !(SREG & 1<<SREG_I) )
#endif

#define HalDisableInterrupts() asm(" cli")
#define HalEnableInterrupts()  asm(" sei")

//Avr defines to help with debug leds.
//Each platform must define HAL_WATCHDOG_MASK variable, but not DDR
#define HAL_WATCHDOG_MASK		PORTC
#define HAL_WATCHDOG_MASK_DDR	DDRC

//usart registers
#define UCSRA   UCSR0A
#define UCSRB   UCSR0B
#define UCSRC   UCSR0C
#define UDR     UDR0
#define UBRR    UBRR0L

#define HalResetClock() (TCNT0 = 0xff-1*16)

void __attribute__((signal)) TIMER0_OVF_vect(void);
#define TimerInterrupt TIMER0_OVF_vect


void HAL_NAKED_FUNCTION HalContextSwitch();

#endif //end if #ifdef AVR_BUILD
//-----------------------------------------------------------------------------

//
//LINUX DEFINES
//
#ifdef PC_BUILD

#if LINUX || BSD

#include<ucontext.h>
#include<stdio.h>
#include<stdlib.h>

/*
 * On System V machines we maintain our machine context using a
 * ucontext_t structure. This sturucture stores the pointer to the
 * context that is resumed, the signals that are blocked by the
 * context, the stack information, and the register states.
 *
 * We can use system V system calls to switch to and from our
 * ucontext_t.
 */

struct MACHINE_CONTEXT
{
	INDEX Flag;//Thread number which gets used for the watchdog and debug leds.
	STACK_INIT_ROUTINE Foo;//Pointer to the first function the thread calls.
	ucontext_t uc;

#ifdef DEBUG
	//Counters to keep track of thread usage.
	COUNT TimesRun;
	COUNT TimesSwitched;
	//Pointers to the top and bottom of the stack. Used to detect stack overflow.
	char * High;
	char * Low;
#endif
};

#endif

#ifdef DARWIN

#include<setjmp.h>
#include<stdlib.h>

/*
 * On Darwin machines we maintain our machine context using a
 * jmp_buf array. This sturucture stores raw register state.
 *
 * We can use setjmp and longjmp to save and restore the register state.
 */

struct MACHINE_CONTEXT
{
	INDEX Flag;//Thread number which gets used for the watchdog and debug leds.
	STACK_INIT_ROUTINE Foo;//Pointer to the first function the thread calls.
	jmp_buf Registers;//Buffer to hold register state in context switches.
	
#ifdef DEBUG
	//Counters to keep track of thread usage.
	COUNT TimesRun;
	COUNT TimesSwitched;
	//Pointers to the top and bottom of the stack. Used to detect stack overflow.
	char * High;
	char * Low;
#endif
};

#endif

#ifdef DEBUG
BOOL HalIsAtomic();
#endif
void HalDisableInterrupts();
void HalEnableInterrupts();
void HalContextSwitch();

extern char HAL_WATCHDOG_MASK;

void HalResetClock();

#endif //end if #ifdef LINUX_BUILD
//-----------------------------------------------------------------------------

#endif //end of #ifndef HAL_H
