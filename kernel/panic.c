#include"panic.h"
#include"hal.h"

void Panic( char file[], int line )
{
        HalPanicFn(file,line, "");
}
