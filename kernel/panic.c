#include"panic.h"
#include"hal.h"

void GeneralPanic( enum ERROR_CODE error )
{
	volatile char failed = 1;
	HalDisableInterrupts();
	while(failed);
	//allows for the machine to contunue in debug.
	HalEnableInterrupts();
}

void KernelPanic( enum ERROR_CODE error )
{
	volatile char failed = 1;
	HalDisableInterrupts();
	while(failed);
	//Allow machine to continue on error
	HalEnableInterrupts();
}
