#ifndef MUTEX_H
#define MUTEX_H

#include"utils/utils.h"

struct MUTEX
{
        ATOMIC Locked;
};

_Bool MutexLock( struct MUTEX * lock );
void MutexUnlock( struct MUTEX * lock );
void MutexInit( struct MUTEX * lock, _Bool initialState );

#ifdef DEBUG
_Bool MutexIsLocked( struct MUTEX * lock );
#endif

#endif
