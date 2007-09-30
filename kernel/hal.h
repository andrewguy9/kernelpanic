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


#define MOTOR_STEP_DIR     PORTD
#define MOTOR_STEP_DIR_DDR DDRD

#define MOTOR_STEP1        _BV(1)
#define MOTOR_STEP2        _BV(3)
#define MOTOR_STEP3        _BV(5)
#define MOTOR_STEP4        _BV(7)

#define MOTOR_DIR1         _BV(0)
#define MOTOR_DIR2         _BV(2)
#define MOTOR_DIR3         _BV(4)
#define MOTOR_DIR4         _BV(6)


#define MOTOR_HOME_MS      PORTE
#define MOTOR_HOME_MS_DDR  DDRE

#define MOTOR_HOME1        _BV(4)
#define MOTOR_HOME2        _BV(5)
#define MOTOR_HOME3        _BV(6)
#define MOTOR_HOME4        _BV(7)

#define MOTOR_MS1          _BV(2)
#define MOTOR_MS2          _BV(3)


#define MISC_SIGNAL        PORTB
#define MISC_SIGNAL_DDR    DDRB

#define MISC_SLEEP         _BV(6)
#define MISC_SENG0         _BV(0)
#define MISC_SENG1         _BV(1)
#define MISC_SENG2         _BV(2)
#define MISC_SENG3         _BV(3)


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

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void);
#define TimerInterrupt TIMER0_OVF_vect

#endif //end if #ifdef AVR_BUILD

//-----------------------------------------------------------------------------

#endif //end of #ifndef HAL_H

