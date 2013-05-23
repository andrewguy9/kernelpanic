#ifndef MUTEX_H
#define MUTEX_H

#include"utils/utils.h"

struct MUTEX
{
        ATOMIC Locked;
};

BOOL MutexLock( struct MUTEX * lock );
void MutexUnlock( struct MUTEX * lock );
void MutexInit( struct MUTEX * lock, BOOL initialState );

#ifdef DEBUG
BOOL MutexIsLocked( struct MUTEX * lock );
#endif

#endif
