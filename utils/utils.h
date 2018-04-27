#ifndef UTILS_H
#define UTILS_H

#include"types.h"

//
//IFDEF DEFINITIONS
//
#ifndef NULL
#define NULL ((void *) 0)
#endif

#define FALSE ((BOOL) 0)
#define TRUE ((BOOL)(! FALSE ))

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

/* Fail: Macro for bailing out on onrecoverable failure.
 * On kernel builds, causes a panic.
 * On app builds calls abort. */
#ifdef APP_BUILD
#include<stdio.h>
#include<stdlib.h>
#define FAIL( msg )                                           \
  do {                                                        \
    printf(msg " in file %s, line %d\n", __FILE__, __LINE__); \
    abort();                                                  \
  } while(0)
#endif //APP_BUILD
#ifdef KERNEL_BUILD
#include"kernel/panic.h"
#define FAIL( msg )               \
  do {                            \
    Panic( __FILE__, __LINE__ );  \
  } while (0)
#endif //KERNEL_BUILD

/* Assert: If condition evaluates to false, fail.
 * Applies only on debug builds. Not evaluated on release builds.
 */
#ifdef DEBUG
#define ASSERT( condition )   \
  do {                        \
    if( !(condition) ) {      \
      FAIL("assert FAILED");  \
    }                         \
  } while (0)
#else // DEBUG
#define ASSERT( condition )
#endif //DEBUG

/* Assume: On debug, evalutes expression and asserts equality with result.
 * On release builds, evaulates expression, with no check of result.
 */
#ifdef DEBUG
#define ASSUME( expression, result )    \
  do {                                  \
    ASSERT( (expression) == (result) ); \
  } while (0)
#else
#define ASSUME( expression, result ) (expression)
#endif //DEBUG

#define FOR_EACH_N(item, array, count)            \
  for(typeof(*(array)) *item = (array),           \
                       *stop = (array) + (count); \
      item != stop;                               \
      item++)

#define FOR_EACH(item, array) FOR_EACH_N(item, array, sizeof (array) / sizeof *(array))
#endif //UTILS_H
