#ifndef MUTEX_H
#define MUTEX_H

#include"../utils/utils.h"

struct MUTEX
{
	BOOL Locked;
};

BOOL MutexLock( struct MUTEX * lock );
void MutexUnlock( struct MUTEX * lock );
BOOL MutexIsLocked( struct MUTEX * lock );
void MutexLockInit( struct MUTEX * lock );

#endif
