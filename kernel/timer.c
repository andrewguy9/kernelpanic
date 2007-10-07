#include"timer.h"
#include"../utils/utils.h"
#include"../utils/heap.h"
#include"hal.h"
#include"scheduler.h"

extern struct THREAD * ActiveThread;
extern struct THREAD * NextThread;


//Keep track of system time for timers.
TIME Time;

//Keep track of timers waiting to execute.
struct HEAP Timers;

void TimerInit( )
{
	Time = 0;
	HeapInit( &Timers );
	HalInitClock();
}

void TimerRegisterASR( struct TIMER * newTimer, 
		TIME wait, 
		TIMER_HANDLER * handler )
{
	ASSERT( HalIsAtomic(), "timer structures can only be added from \
			interrupt level");
	ASSERT( newTimer != NULL, "null argument" );
	ASSERT( handler != NULL, "null handler" );

	newTimer->Link.Weight = Time + wait;
	newTimer->Handler = handler;
	HeapAdd( (struct WEIGHTED_LINK * ) newTimer, &Timers );
}

void RunTimers( )
{
	ASSERT( HalIsAtomic(), "timers can only be run from interrupt level.");
	Time++;
	while( HeapSize( &Timers) > 0 && 
			HeapHeadWeight( &Timers ) >= Time )
	{
		struct TIMER * timer = (struct TIMER *) HeapPop( & Timers );
		timer->Handler();
	}
}

void __attribute__((naked,signal,__INTR_ATTRS)) TIMER0_OVF_vect(void) 
{
	//TODO: Make this function only context switch when needed.
	//PLAN: Map out and only push registers that we _need_ unil we
	//know there was a context change. 
	

	//Backup registers
	    //Save the state
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

	//Save the stack pointer
    ActiveThread->Stack = (void *) SP;


	//Call the timer utility
	RunTimers( );

	//reset the clock
    TCNT0 = 0xff-1*16; //1 ms

	//Check for scheduling event
	if( NextThread != NULL )
	{
		ActiveThread = NextThread;
		NextThread = NULL;
	}

	//Restore stack
	SP = (int) ActiveThread->Stack;

	//restore registers
	//restore the state
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

