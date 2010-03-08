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
#define MAX( A, B ) ((A)>(B) ? (A) : (B) )
#define MIN( A, B ) ((A)<(B) ? (A) : (B) )
#define ASSENDING( A, B, C ) ((A) <= (B) && (B) <= (C))
//Returns the byte offset of FIELD in TYPE
#define OFFSET_OF( TYPE, FIELD ) ((INDEX)(&(((TYPE *)0)->FIELD)))
//Returns a pointer to the base structure 
//given a pointer to a field.
#define BASE_OBJECT( PTR, BASE, FIELD ) ((BASE*)((INDEX)(PTR) - OFFSET_OF(BASE,FIELD)))

//
//  Assert and Assume
//

#ifdef DEBUG

#ifdef APP_BUILD

//This is a app build. Asserts result in printf/exit.
#include<stdio.h>
#include<stdlib.h>
#define ASSERT( condition ) \
	if( !(condition) )      \
		printf("assert FAILED in file %s, line %d\n", __FILE__, __LINE__)

//This is a app build. Assumes result in a app and printf/exit.
#define ASSUME( expression, result ) (ASSERT( (condition) == (result) ))

#endif //APP_BUILD

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
