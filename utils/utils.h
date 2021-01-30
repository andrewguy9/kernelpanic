#ifndef UTILS_H
#define UTILS_H
#include"types.h"
#include<stdbool.h>
#include<stdint.h>
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
//  Assert and Check
//

#ifdef DEBUG

#ifdef APP_BUILD

//This is a app build. Asserts result in printf/exit.
#include<stdio.h>
#include<stdlib.h>
#define ASSERT( condition ) \
  ((void)((condition) ? \
   0 : \
   printf("assert FAILED in file %s, line %d\n", __FILE__, __LINE__)))

//This is a app build. CHECKs result in a app and printf/exit.
#define CHECK( expression ) ((void) (ASSERT (expression) ))

#endif //APP_BUILD

#ifdef KERNEL_BUILD

//This is a kernel build. Asserts result in a kernel panic.
#include"kernel/panic.h"
#define ASSERT( condition ) \
    ((void)((condition) ? \
     0 : \
     Panic( __FILE__, __LINE__ )))

#define CHECK( expression ) ((void) ASSERT(expression))

#endif //ifdef KERNEL_BUILD

#else //ifdef DEBUG

//This is a fre build, no asserts enabled.
#define ASSERT( condition ) ((void) (NULL))

//This is a fre build, CHECK runs expression, but no ASSERT
#define CHECK( expression ) ((void)(expression))

#endif //ifdef DEBUG


#endif
