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
 * Disables interrupts and enters busy loop. Call for errors inside
 * of kernel. 
 */
void Panic( char file[], int line )
{
	volatile char failed = 1;
	InterruptDisable();
	while(failed);
	//Allow machine to continue on error
	InterruptEnable();
}
