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

//
//  Assert
//

#ifdef DEBUG

#ifdef TEST_BUILD

//This is a test build. Asserts result in printf/exit.
#include<stdio.h>
#include<stdlib.h>
#define ASSERT( condition ) \
	if( !(condition) )      \
		printf("assert FAILED in file %s, line %d\n", __FILE__, __LINE__)

#endif //TEST_BUILD

#ifdef KERNEL_BUILD

//This is a kernel build. Asserts result in a kernel panic.
#include"../kernel/panic.h"
#define ASSERT( condition ) \
	if( ! (condition) ) \
		Panic( __FILE__, __LINE__ )

#endif //ifdef KERNEL_BUILD

#else //ifdef DEBUG

//This is a fre build, no asserts enabled.
#define ASSERT( condition ) 

#endif //ifdef DEBUG


#endif
