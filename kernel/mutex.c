#include"mutex.h"
#include"hal.h"

/*
 * The mutex library is a thread and asr safe syncronization mechanism.
 */


/*
 * MutexLock attempts to lock the lock mutex, by disabling interrupts
 * and then checking the lock's state. The return value is true if
 * the mutex was aquired or false if the call failed ot aquire the lock.
 */
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

/*
 * Unlocks the mutex.
 */
void MutexUnlock( struct MUTEX * lock )
{
	HalDisableInterrupts();
	ASSERT( lock->Locked == TRUE, "Tried to unlock an unlocked mutex");
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
