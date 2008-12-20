#ifndef HAL_H
#define HAL_H

//Includes for all builds
#include"../utils/utils.h"
#include"../utils/flags.h"
//-----------------------------------------------------------------------------
//Defines (for all builds)
typedef void (*STACK_INIT_ROUTINE) ();
//-----------------------------------------------------------------------------
//Prototypes ( For all Builds )
struct MACHINE_CONTEXT;
void HalInitClock();
void HalStartup();
void HalStartInterrupt();
void HalEndInterrupt();
void HalCreateStackFrame( struct MACHINE_CONTEXT * Context, void * stack, STACK_INIT_ROUTINE foo, COUNT stackSize);
void HalGetInitialStackFrame( struct MACHINE_CONTEXT * Context );
void HalSerialStartup();
void HalPanic(char file[], int line);
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

#define HAL_NAKED_FUNCTION __attribute__((naked,__INTR_ATTRS))
struct MACHINE_CONTEXT
{
	char * Stack;

#ifdef DEBUG
	char * High;
	char * Low;
#endif
};

//Macros to change machine interrupt state.
#define HalIsAtomic() ( !(SREG & 1<<SREG_I) )
#define HalDisableInterrupts() asm(" cli")
#define HalEnableInterrupts()  asm(" sei")

//Avr defines to help with debug leds.
//Each platform must define DEBUG_LED variable, but not DDR
#define DEBUG_LED          PORTC
#define DEBUG_LED_DDR      DDRC

//usart registers
#define UCSRA   UCSR0A
#define UCSRB   UCSR0B
#define UCSRC   UCSR0C
#define UDR     UDR0
#define UBRR    UBRR0L

#define HalResetClock() (TCNT0 = 0xff-1*16)

void __attribute__((signal,__INTR_ATTRS)) TIMER0_OVF_vect(void);
#define TimerInterrupt TIMER0_OVF_vect


void HAL_NAKED_FUNCTION HalContextSwitch();

#endif //end if #ifdef AVR_BUILD
//-----------------------------------------------------------------------------

//
//LINUX DEFINES
//
#ifdef PC_BUILD

#include<setjmp.h>
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

	STACK_INIT_ROUTINE Foo;
	sigjmp_buf Registers;

#ifdef DEBUG
	char * High;
	char * Low;
#endif
};

BOOL HalIsAtomic();
void HalDisableInterrupts();
void HalEnableInterrupts();
void HalContextSwitch();

extern char DEBUG_LED;

void HalResetClock();

#endif //end if #ifdef LINUX_BUILD
//-----------------------------------------------------------------------------

//
//Cross Platform Macros
//

#define HalSetDebugLedMask( mask ) ( DEBUG_LED = mask )
#define HalSetDebugLedFlag( index ) ( FlagOn( &(DEBUG_LED),(index)) )
#define HalClearDebugLedFlag( index ) (FlagOff(&(DEBUG_LED),(index)))
#define HalToggleDebugLedFlag( index ) (FlagToggle(&(DEBUG_LED),(index)))
#define HalGetDebugLed( ) ( DEBUG_LED )

#endif //end of #ifndef HAL_H
