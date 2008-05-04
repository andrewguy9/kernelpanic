#include"panic.h"
#include"interrupt.h"

/*
 * Panic Unit Description:
 * When the system encounters an error it sould call a panic function. 
 * We support two types of panic. 
 * GeneralPanic is for thread panicing and Kernel is for kernel 
 * (and isr) panicing. The panic routines poll on their local failed
 * variables until a debugger is used to set it to zero. Using this 
 * strategy you can continue running the program in the debugger.
 */

/*
 * Panic for threads. In the future this will kill all the user threads.
 * leaving the kernel threads report the error.
 */
void GeneralPanic( enum ERROR_CODE error )
{
	//TODO MAKE THIS DO WORK, CURRENTLY SAME AS KernelPanic
	volatile char failed = 1;
	InterruptDisable();
	while(failed);
	//allows for the machine to contunue in debug.
	InterruptEnable();
}

/*
 * Disables interrupts and enters busy loop. Call for errors inside
 * of kernel. 
 */
void KernelPanic( enum ERROR_CODE error )
{
	volatile char failed = 1;
	InterruptDisable();
	while(failed);
	//Allow machine to continue on error
	InterruptEnable();
}
