#ifndef TYPES_H
#define TYPES_H

//
//TYPEDEFS
//

#ifdef AVR_BUILD
typedef unsigned int COUNT;
typedef unsigned int INDEX;
typedef unsigned char BOOL;
typedef unsigned long int TIME;
typedef unsigned char ATOMIC;
#endif

#ifdef PC_BUILD
typedef unsigned long int COUNT;
typedef unsigned long int INDEX;
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

ATOMIC HalAtomicGetAndAnd(ATOMIC * var, ATOMIC val);
ATOMIC HalAtomicGetAndOr(ATOMIC * var, ATOMIC val);
ATOMIC HalCompareAndSwap(ATOMIC * var, ATOMIC old, ATOMIC next);
DOUBLE_COUNT HalDoubleCompareAndSwap(DOUBLE_COUNT * var, DOUBLE_COUNT old, DOUBLE_COUNT next);

#define HalAtomicGetAndSet(var) (HalAtomicGetAndOr(var, 1))
#define HalAtomicGetAndClear(var) (HalAtomicGetAndAnd(var, 0))
#define HalCompareAndSwapPtrs(var, old, next) \
        HalCompareAndSwap((ATOMIC *) (var), (ATOMIC) (old), (ATOMIC) (next))
#endif // TYPES_H
