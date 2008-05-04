#include"mutex.h"
#include"interrupt.h"

/*
 * MUTEX UNIT DESCRIPTION
 * The mutex library is a thread and asr safe primative syncronization mechanism.
 *
 * Mutexes should be used whenever threads and asr's need to access the same data.
 * Be aware that mutexes do not support blocking. This means that they are NOT starvation
 * safe. If a unit polls tightly on the mutex it could hold it FOREVER, or it may not ever 
 * acquire the lock.
 *
 * To make sure this does not happen only use mutexes when usage of the lock is gauranteed
 * to be sparce. 
 */


/*
 * MutexLock attempts to lock the lock mutex, by disabling interrupts
 * and then checking the lock's state. The return value is true if
 * the mutex was aquired or false if the call failed ot aquire the lock.
 */
BOOL MutexLock( struct MUTEX * lock )
{
	BOOL success;
	InterruptDisable();
	if( lock->Locked )
	{
		success = FALSE;
	}
	else
	{
		success = TRUE;
		lock->Locked = TRUE;
	}
	InterruptEnable();
	return success;
}

/*
 * Unlocks the mutex.
 */
void MutexUnlock( struct MUTEX * lock )
{
	InterruptDisable();
	ASSERT( lock->Locked == TRUE,
			MUTEX_UNLOCK_WHILE_UNLOCKED,
		   	"Tried to unlock an unlocked mutex");
	lock->Locked = FALSE;
	InterruptEnable();
}

/*
 * Returns the state of the lock:
 * FALSE - Not Locked
 * !FALSE - Locked
 */
BOOL MutexIsLocked( struct MUTEX * lock )
{
	BOOL value;
	InterruptDisable();
	value = lock->Locked;
	InterruptEnable();
	return value;
}

/*
 * Initializes a mutex to unlocked.
 */
void MutexInit( struct MUTEX * lock )
{
	lock->Locked = FALSE; 
}
