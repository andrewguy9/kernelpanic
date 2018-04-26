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

// https://stackoverflow.com/questions/400951/does-c-have-a-foreach-loop-construct
#if 0
#define FOR_EACH(item, array) \
  for(int keep = 1, \
      count = 0,\
      size = sizeof (array) / sizeof *(array); \
      keep && count != size; \
      keep = !keep, count++) \
      for(typeof(array[0]) * item = (array) + count; keep; keep = !keep)
#else
#define FOR_EACH(item, array)                                            \
  for(typeof(*(array)) *item = array,                                    \
                       *stop = array + sizeof (array) / sizeof *(array); \
      item != stop;                                                      \
      item++)
#endif
#endif //UTILS_H
