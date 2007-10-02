#ifndef HAL_H
#define HAL_H

//-----------------------------------------------------------------------------
//Prototypes ( For all Builds )
void HalInitClock();
//-----------------------------------------------------------------------------

#ifdef AVR_BUILD

#define GET_SP() ((void *) SP)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#define TMR_PRESCALE_NONE  0xF4
#define TMR_PRESCALE_1024  0x07
#define TMR_PRESCALE_MASK  0x07
#define TMR_MS             16

#define DEBUG_LED          PORTC
#define DEBUG_LED_DDR      DDRC

#define DEBUG_SW           PINA
#define DEBUG_SW_PORT      PORTA
#define DEBUG_SW_DDR       DDRA


/*
 * Disables and enables all interrupts globally.
 * This is to be used as a strong critial section
 * and only used for very very short sections.
 */
#define DISABLE_INTERRUPTS() asm(" cli")
#define ENABLE_INTERRUPTS() asm(" sei")

//Redefinition of DISABLE_INTERRUPTS and ENABLE_INTERRUPTS for ease of use. 
#define START_CRITICAL() DISABLE_INTERRUPTS()
#define END_CRITICAL() ENABLE_INTERRUPTS()

#define IS_ATOMIC() (SREG&SREG_I)

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void);
#define TimerInterrupt TIMER0_OVF_vect

#endif //end if #ifdef AVR_BUILD

//-----------------------------------------------------------------------------

#endif //end of #ifndef HAL_H

