#include"panic.h"
#include"hal.h"

void GeneralPanic( enum ERROR_CODE error )
{
	HalDisableInterrupts();
	while(1);
}

void KernelPanic( enum ERROR_CODE error )
{
	HalDisableInterrupts();
	while(1);
}
