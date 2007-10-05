#include"mutex.h"
#include"hal.h"

BOOL MutexLock( struct MUTEX * lock )
{
	BOOL success;
	HalDisableInterrupts();
	if( lock->Locked )
	{
		success = FALSE;
	}
	else
	{
		success = TRUE;
		lock->Locked = TRUE;
	}
	HalEnableInterrupts();
	return success;
}

void MutexUnlock( struct MUTEX * lock )
{
	HalDisableInterrupts();
	lock->Locked = FALSE;
	HalEnableInterrupts();
}

BOOL MutexIsLocked( struct MUTEX * lock )
{
	BOOL value;
	HalDisableInterrupts();
	value = lock->Locked;
	HalEnableInterrupts();
	return value;
}

void MutexLockInit( struct MUTEX * lock )
{
	lock->Locked = FALSE; 
}
