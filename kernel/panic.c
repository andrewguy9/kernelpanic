#include"panic.h"
#include"hal.h"

void GeneralPanic( enum ERROR_CODE error )
{
	volatile char failed = 1;
	HalDisableInterrupts();
	while(failed);
}

void KernelPanic( enum ERROR_CODE error )
{
	volatile char failed = 1;
	HalDisableInterrupts();
	while(failed);
}
