#define HAL_MIN_STACK_SIZE MINSIGSTKSZ

#include<setjmp.h>
#include<stdlib.h>
#include<signal.h>

/*
 * On System V we can use sigaltstack and setjmp, longjmp to boostrap new user
 * threads without the need for machine specific code.
 *
 * For context switches we use setjmp to save the current register values 
 * and longjmp to jump to a saved register context.
 *
 * To bootstrap a thread, we call sigaltstack to set the signal stack for 
 * SIGUSR1 to the desired new thread stack. Then we fire off a SIGUSR1 
 * signal. This will cause the kernel to switch the stack pointer to the 
 * alternate stack. When we wake in the HalStackTrampoline function we 
 * know that we are trying to bootstrap a new thread.
 *
 * HalStackTrampoline saves his context by calling setjmp and returns.
 * Later when the new thread is picked as the next thread a call to longjmp
 * will cause the new thread to wake from setjmp. We check the return code,
 * realize we have just been scheduled and call the kernel's thraed bootstrap 
 * function.
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

#ifdef DEBUG
BOOL HalIsAtomic();
#endif

void HalDisableInterrupts();
void HalEnableInterrupts();
void HalContextSwitch();

extern char HalWatchdogMask;

void HalResetClock();

