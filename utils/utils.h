#ifndef UTILS_H
#define UTILS_H

#include"types.h"
#include"stdbool.h"
#include"stddef.h"

//
//MATH MACROS
//
#define MAX( a, b ) ((a)>(b) ? (a) : (b) )
#define MIN( a, b ) ((a)<(b) ? (a) : (b) )
#define CLAMP(value, max, min) (MIN(MAX(min, value), max))
#define ASSENDING( a, b, c ) ((a) <= (b) && (b) <= (c))
//Returns a pointer to the base structure 
//given a pointer to a field.
#define container_of( ptr, base, field ) ((base*)((INDEX)(ptr) - offsetof(base,field)))

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
#include"kernel/panic.h"
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
