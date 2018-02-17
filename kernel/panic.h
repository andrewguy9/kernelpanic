#ifndef PANIC_H
#define PANIC_H

void Panic( char file[], int line )  __attribute__ ((noreturn));

#define GeneralPanic( ) Panic( __FILE__, __LINE__ )
#define KernelPanic( ) Panic( __FILE__, __LINE__ )

#endif
