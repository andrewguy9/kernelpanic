#include"types.h"

#ifdef PC_BUILD
ATOMIC HalAtomicGetAndOr(ATOMIC * var, ATOMIC val)
{
        return __sync_fetch_and_or(var, val);
}

ATOMIC HalAtomicGetAndAnd(ATOMIC * var, ATOMIC val)
{
        return __sync_fetch_and_and(var, val);
}

ATOMIC HalCompareAndSwap(ATOMIC * var, void * old, void * next)
{
        return __sync_val_compare_and_swap(var, old, next);
}

#endif
