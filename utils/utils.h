#ifndef UTILS_H
#define UTILS_H


//
//IFDEF DEFINITIONS
//
#ifndef NULL
#define NULL 0
#endif

#define FALSE ((BOOL) 0)
#define TRUE ((BOOL)(! FALSE ))

//
//PC BUILD DEFINITIONS
//
#ifdef PC_BUILD

#include<stdio.h>
#include<stdlib.h>
#define ASSERT( condition, errornum, errorstr ) if( ! (condition) ) {\
		printf("ASSERT FAILED: %s\n", errorstr); exit(0); } \
		printf("dead\n")

#endif

//
//AVR BUILD DEFINITIONS
//
#ifdef AVR_BUILD
#include"../kernel/panic.h"
#define ASSERT( condition, errornum, errorstr ) \
	if( ! (condition) ) \
		KernelPanic( errornum )

#endif


//
//TYPEDEFS
//
typedef unsigned int COUNT;
typedef unsigned int INDEX;
typedef unsigned char BOOL;


//
//MATH MACROS
//
#define MAX( A, B ) ((A)>(B) ? (A) : (B) )
#define MIN( A, B ) ((A)<(B) ? (A) : (B) )
#define ASSENDING( A, B, C ) ((A) <= (B) && (B) <= (C))
//Returns the byte offset of FIELD in TYPE
#define OFFSET_OF( TYPE, FIELD ) ((int)(&(((TYPE *)0)->FIELD)))
//Returns a pointer to the base structure 
//given a pointer to a field.
#define BASE_OBJECT( PTR, BASE, FIELD ) ((BASE*)((int)(PTR) - OFFSET_OF(BASE,FIELD)))
#endif
