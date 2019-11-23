#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>

//
//TYPEDEFS
//

typedef size_t COUNT;
typedef size_t INDEX;

#ifdef AVR_BUILD
typedef unsigned char BOOL;
typedef unsigned long int TIME;
typedef unsigned char ATOMIC;
#endif

#ifdef PC_BUILD
typedef unsigned char BOOL;
typedef unsigned long int TIME;
typedef unsigned long int ATOMIC;
typedef unsigned long long DOUBLE_COUNT;
#endif

struct ATOMIC_TUPLE
{
        void * Pointer;
        COUNT Generation;
};

union ATOMIC_UNION
{
        DOUBLE_COUNT Atomic;
        struct ATOMIC_TUPLE Tuple;
};

ATOMIC AtomicGetAndAnd(ATOMIC * var, ATOMIC val);
ATOMIC AtomicGetAndOr(ATOMIC * var, ATOMIC val);
ATOMIC CompareAndSwap(ATOMIC * var, ATOMIC old, ATOMIC next);
DOUBLE_COUNT DoubleCompareAndSwap(DOUBLE_COUNT * var, DOUBLE_COUNT old, DOUBLE_COUNT next);

#define AtomicGetAndSet(var) (AtomicGetAndOr(var, 1))
#define AtomicGetAndClear(var) (AtomicGetAndAnd(var, 0))
#define CompareAndSwapPtrs(var, old, next) \
        CompareAndSwap((ATOMIC *) (var), (ATOMIC) (old), (ATOMIC) (next))
#endif // TYPES_H
