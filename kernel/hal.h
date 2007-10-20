#ifndef HAL_H
#define HAL_H

//Includes for all builds
#include"../utils/utils.h"
#include"scheduler.h"
//-----------------------------------------------------------------------------
//Prototypes ( For all Builds )
void HalInitClock();
BOOL HalIsAtomic();
void HalInit();
void HalEnableInterrupts();
void HalDisableInterrupts();
void HalStartInterrupt();
void HalEndInterrupt();
void HalCreateStackFrame( struct THREAD * thread, THREAD_MAIN main );
//-----------------------------------------------------------------------------

//
//AVR DEFINES
//
#ifdef AVR_BUILD

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#define HAL_SAVE_SP(dest) (dest = (void *) SP)
#define HAL_SET_SP(value) (SP = (int) (value) )

#define HalIsAtomic() ( !(SREG&SREG_I) )
#define HAL_DISABLE_INTERRUPTS() asm(" cli")
#define HAL_ENABLE_INTERRUPTS() asm(" sei")

#define TMR_PRESCALE_NONE  0xF4
#define TMR_PRESCALE_1024  0x07
#define TMR_PRESCALE_MASK  0x07
#define TMR_MS             16

#define DEBUG_LED          PORTC
#define DEBUG_LED_DDR      DDRC

#define DEBUG_SW           PINA
#define DEBUG_SW_PORT      PORTA
#define DEBUG_SW_DDR       DDRA

//void inline HalRestoreState();
//void inline HalSaveState();

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
			"reti\n\t" \
			);

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void);
#define TimerInterrupt TIMER0_OVF_vect

#endif //end if #ifdef AVR_BUILD
//-----------------------------------------------------------------------------

#endif //end of #ifndef HAL_H
