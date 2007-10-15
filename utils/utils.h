#ifndef UTILS_H
#define UTILS_H

#ifndef NULL
#define NULL 0
#endif

#define FALSE 0
#define TRUE (! FALSE )

#ifdef PC_BUILD

#include<stdio.h>
#include<stdlib.h>
#define ASSERT( condition, errornum, error ) ( condition ? :  \
		printf("ASSERT FAILED: %s\n", error))

#endif

#ifdef AVR_BUILD
#include"../kernel/panic.h"
#define ASSERT( condition, errornum, error ) ( condition ? : \
		KernelPanic( errornum ) )

#endif

typedef unsigned int COUNT;
typedef unsigned int INDEX;
typedef unsigned char BOOL;

#endif
