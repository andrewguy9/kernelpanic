#include"types.h"

#ifdef PC_BUILD
ATOMIC AtomicGetAndOr(ATOMIC * var, ATOMIC val)
{
        return __sync_fetch_and_or(var, val);
}

ATOMIC AtomicGetAndAnd(ATOMIC * var, ATOMIC val)
{
        return __sync_fetch_and_and(var, val);
}

ATOMIC CompareAndSwap(ATOMIC * var, ATOMIC old, ATOMIC next)
{
        return __sync_val_compare_and_swap(var, old, next);
}

DOUBLE_COUNT DoubleCompareAndSwap(DOUBLE_COUNT * var, DOUBLE_COUNT old, DOUBLE_COUNT next)
{
        return __sync_val_compare_and_swap(var, old, next);
}

#endif
