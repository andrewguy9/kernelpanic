#include"mutex.h"
#include"isr.h"

/*
 * MUTEX UNIT DESCRIPTION
 * The mutex library is a thread and ISR safe primitive synchronization mechanism.
 * Mutexes are consistant at any IRQ, so they can be used for syncronization between
 * different IRQ levels.
 *
 * Mutexes should be used whenever threads and ISRs need to access the same data.
 * Be aware that mutexes do not support blocking. This means that they are NOT starvation
 * safe. If a unit polls tightly on the mutex it could hold it FOREVER, or it may not ever 
 * acquire the lock.
 *
 * To make sure this does not happen only use mutexes when usage of the lock is gauranteed
 * to be sparse. 
 */


/*
 * MutexLock attempts to lock the lock mutex, by disabling interrupts
 * and then checking the lock's state. The return value is true if
 * the mutex was aquired or false if the call failed ot aquire the lock.
 */
BOOL MutexLock( struct MUTEX * lock )
{
	BOOL success;
	IsrDisable(IRQ_LEVEL_MAX);
	if( lock->Locked )
	{
		success = FALSE;
	}
	else
	{
		success = TRUE;
		lock->Locked = TRUE;
	}
	IsrEnable(IRQ_LEVEL_MAX);
	return success;
}

/*
 * Unlocks the mutex.
 */
void MutexUnlock( struct MUTEX * lock )
{
	IsrDisable(IRQ_LEVEL_MAX);
	ASSERT( lock->Locked == TRUE );
	lock->Locked = FALSE;
	IsrEnable(IRQ_LEVEL_MAX);
}

#ifdef DEBUG
/*
 * Returns the state of the lock:
 * FALSE - Not Locked
 * !FALSE - Locked
 */
BOOL MutexIsLocked( struct MUTEX * lock )
{
	BOOL value;
	IsrDisable(IRQ_LEVEL_MAX);
	value = lock->Locked;
	IsrEnable(IRQ_LEVEL_MAX);
	return value;
}
#endif

/*
 * Initializes a mutex to unlocked.
 */
void MutexInit( struct MUTEX * lock, BOOL intialState )
{
	lock->Locked = intialState; 
}
