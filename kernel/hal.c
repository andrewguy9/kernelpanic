#include "hal.h"

void HalInitClock()
{
    /*
	 * TODO VERIFY THAT THE TIMER IS CORRECTLY SET TO 1 MILLISECOND
	 * 8-bit timer should have clk/1024
	 * 15.625 counts is 1ms @ clk/1024
     */

	//preload the timer counter
	TCNT0 = 0xff-1*TMR_MS; //1 ms

	//enable interrupts
	TIMSK |= _BV(TOIE0);

	//set clock source to start timer
	TCCR0 |= TMR_PRESCALE_1024;
}

//-----------------------------------------------------------------------------
BOOL HalIsAtomic()
{
	return SREG&SREG_I;
}

COUNT InterruptLevel;
void HalDisableInterrupts()
{
	asm(" cli");
	InterruptLevel++;
}

void HalEnableInterrupts()
{
	InterruptLevel--;
	if( InterruptLevel == 0 )
		asm(" sei");
}

void HalPrepareRETI()
{
	ASSERT( InterruptLevel <= 1, 
			"Interrupt level cannot be greater than 1" );
	InterruptLevel = 0;
}

void HalInit()
{
	InterruptLevel = 1;
}

void inline HalSaveState()
{
	asm( 
			"push r0\n\t"
			"push r1\n\t"
			"push r2\n\t"
			"push r3\n\t"
			"push r4\n\t"
			"push r5\n\t"
			"push r6\n\t"
			"push r7\n\t"
			"push r8\n\t"
			"push r9\n\t"
			"push r10\n\t"
			"push r11\n\t"
			"push r12\n\t"
			"push r13\n\t"
			"push r14\n\t"
			"push r15\n\t"
			"push r16\n\t"
			"push r17\n\t"
			"push r18\n\t"
			"push r19\n\t"
			"push r20\n\t"
			"push r21\n\t"
			"push r22\n\t"
			"push r23\n\t"
			"push r24\n\t"
			"push r25\n\t"
			"push r26\n\t"
			"push r27\n\t"
			"push r28\n\t"
			"push r29\n\t"
			"push r30\n\t"
			"push r31\n\t"
			"in r0, 0x3F\n\t"
			"push r0\n\t"
			);
}

void inline HalRestoreState()
{
	asm( 
            "pop r0\n\t"
            "out 0x3F, r0\n\t"
            "pop r31\n\t"
            "pop r30\n\t"
            "pop r29\n\t"
            "pop r28\n\t"
            "pop r27\n\t"
            "pop r26\n\t"
            "pop r25\n\t"
            "pop r24\n\t"
            "pop r23\n\t"
            "pop r22\n\t"
            "pop r21\n\t"
            "pop r20\n\t"
            "pop r19\n\t"
            "pop r18\n\t"
            "pop r17\n\t"
            "pop r16\n\t"
            "pop r15\n\t"
            "pop r14\n\t"
            "pop r13\n\t"
            "pop r12\n\t"
            "pop r11\n\t"
            "pop r10\n\t"
            "pop r9\n\t"
            "pop r8\n\t"
            "pop r7\n\t"
            "pop r6\n\t"
            "pop r5\n\t"
            "pop r4\n\t"
            "pop r3\n\t"
            "pop r2\n\t"
            "pop r1\n\t"
            "pop r0\n\t"
			"reti\n\t"
			);
}
