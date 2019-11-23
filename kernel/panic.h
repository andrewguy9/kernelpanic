#ifndef PANIC_H
#define PANIC_H

void Panic( char file[], int line );

#define KernelPanic( ) Panic( __FILE__, __LINE__ )

#endif
