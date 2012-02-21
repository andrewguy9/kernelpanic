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
#endif
