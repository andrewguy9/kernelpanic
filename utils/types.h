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
#endif

ATOMIC HalAtomicGetAndAnd(ATOMIC * var, ATOMIC val);
ATOMIC HalAtomicGetAndOr(ATOMIC * var, ATOMIC val);
ATOMIC HalCompareAndSwap(ATOMIC * var, void * old, void * next);

#define HalAtomicGetAndSet(var) (HalAtomicGetAndOr(var, 1))
#define HalAtomicGetAndClear(var) (HalAtomicGetAndAnd(var, 0))
#define HalCompareAndSwapPointers(var, old, next) \
        ((void*) HalCompareAndSwap( \
                (void*)(var),       \
                (old),       \
                (next)))

#endif // TYPES_H
