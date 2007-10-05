#ifndef HAL_H
#define HAL_H

//Includes for all builds
#include"../utils/utils.h"
//-----------------------------------------------------------------------------
//Prototypes ( For all Builds )
void HalInitClock();
BOOL HalIsAtomic();
void HalInit();
void HalEnableInterrupts();
void HalDisableInterrupts();
//-----------------------------------------------------------------------------
#ifdef AVR_BUILD

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#define GET_SP() ((void *) SP)

#define TMR_PRESCALE_NONE  0xF4
#define TMR_PRESCALE_1024  0x07
#define TMR_PRESCALE_MASK  0x07
#define TMR_MS             16

#define DEBUG_LED          PORTC
#define DEBUG_LED_DDR      DDRC

#define DEBUG_SW           PINA
#define DEBUG_SW_PORT      PORTA
#define DEBUG_SW_DDR       DDRA

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void);
#define TimerInterrupt TIMER0_OVF_vect

#endif //end if #ifdef AVR_BUILD
//-----------------------------------------------------------------------------
#endif //end of #ifndef HAL_H
