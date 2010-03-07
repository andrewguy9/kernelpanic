#ifndef UTILS_H
#define UTILS_H


//
//IFDEF DEFINITIONS
//
#ifndef NULL
#define NULL ((void *) 0)
#endif

#define FALSE ((BOOL) 0)
#define TRUE ((BOOL)(! FALSE ))

//
//TYPEDEFS
//
#ifdef AVR_BUILD
typedef unsigned int COUNT;
typedef unsigned int INDEX;
typedef unsigned char BOOL;
#endif

#ifdef PC_BUILD
typedef unsigned long int COUNT;
typedef unsigned long int INDEX;
typedef unsigned char BOOL;
#endif

//
//MATH MACROS
//
#define MAX( a, b ) ((a)>(b) ? (a) : (b) )
#define MIN( a, b ) ((a)<(b) ? (a) : (b) )
#define CLAMP(value, max, min) (MIN(MAX(min, value), max))
#define ASSENDING( a, b, c ) ((a) <= (b) && (b) <= (c))
//Returns the byte offset of FIELD in TYPE
#define OFFSET_OF( type, field ) ((INDEX)(&(((type *)0)->field)))
//Returns a pointer to the base structure 
//given a pointer to a field.
#define BASE_OBJECT( ptr, base, field ) ((base*)((INDEX)(ptr) - OFFSET_OF(base,field)))

//
//  Assert and Assume
//

#ifdef DEBUG

#ifdef TEST_BUILD

//This is a test build. Asserts result in printf/exit.
#include<stdio.h>
#include<stdlib.h>
#define ASSERT( condition ) \
	if( !(condition) )      \
		printf("assert FAILED in file %s, line %d\n", __FILE__, __LINE__)

//This is a test build. Assumes result in a test and printf/exit.
#define ASSUME( expression, result ) (ASSERT( (condition) == (result) ))

#endif //TEST_BUILD

#ifdef KERNEL_BUILD

//This is a kernel build. Asserts result in a kernel panic.
#include"../kernel/panic.h"
#define ASSERT( condition ) \
	if( ! (condition) ) \
		Panic( __FILE__, __LINE__ )

#define ASSUME( expression, result ) ASSERT(expression == result )

#endif //ifdef KERNEL_BUILD

#else //ifdef DEBUG

//This is a fre build, no asserts enabled.
#define ASSERT( condition ) 

//This is a fre build, ASSUME runs expression, but no check.
#define ASSUME( expression, result ) (expression)

#endif //ifdef DEBUG


#endif
