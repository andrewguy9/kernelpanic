//
//AVR DEFINES
//

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

void __attribute__((signal,__INTR_ATTRS)) TIMER0_OVF_vect(void);
#define TimerInterrupt TIMER0_OVF_vect


void HAL_NAKED_FUNCTION HalContextSwitch();

