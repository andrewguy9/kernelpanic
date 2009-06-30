//-----------------------------------------------------------------------------
//****************************LINUX AND BSD DEFINES****************************
//-----------------------------------------------------------------------------

#if LINUX || BSD || DARWIN

//_XOPEN_SOURCE only needed for BSD derived kernels.
#if BSD || DARWIN
#define _XOPEN_SOURCE
#endif

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
#endif // DEBUG
};

#endif // LINUX || BSD

//-----------------------------------------------------------------------------
//********************************DARWIN ONLY**********************************
//-----------------------------------------------------------------------------

#ifdef BOGUS

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

#endif // DARWIN

//-----------------------------------------------------------------------------
//*********************************COMMON UNIX*********************************
//-----------------------------------------------------------------------------


#ifdef DEBUG
BOOL HalIsAtomic();
#endif

void HalDisableInterrupts();
void HalEnableInterrupts();
void HalContextSwitch();

extern char HalWatchdogMask;

void HalResetClock();

