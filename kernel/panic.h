#ifndef PANIC_H
#define PANIC_H

void Panic( char * file, int line );

#define GeneralPanic( ) Panic( __FILE__, __LINE__ )
#define KernelPanic( ) Panic( __FILE__, __LINE__ )
#endif
