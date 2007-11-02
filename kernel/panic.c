#include"panic.h"
#include"interrupt.h"

void GeneralPanic( enum ERROR_CODE error )
{
	volatile char failed = 1;
	InterruptDisable();
	while(failed);
	//allows for the machine to contunue in debug.
	InterruptEnable();
}

void KernelPanic( enum ERROR_CODE error )
{
	volatile char failed = 1;
	InterruptDisable();
	while(failed);
	//Allow machine to continue on error
	InterruptEnable();
}
