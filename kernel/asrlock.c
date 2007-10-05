#include"asrlock.h"
#include"hal.h"

BOOL ASRLock( ASR_LOCK * lock )
{
	BOOL success;
	DISABLE_INTERRUPTS();
	if( lock->Locked )
	{
		success = FALSE;
	}
	else
	{
		success = TRUE;
		lock->Locked = TRUE;
	}
	ENABLE_INTERRUPTS();
	return success;
}

void ASRUnlock( ASR_LOCK * lock )
{
	DISABLE_INTERRUPTS();
	lock->Locked = FALSE;
	ENABLE_INTERRUPTS();
}

BOOL ASRIsLocked( ASR_LOCK * lock )
{
	BOOL value;
	DISABLE_INTERRUPTS();
	value = lock->Locked;
	ENABLE_INTERUPTS();
	return value;
}

void ASRLockInit( ASR_LOCK * lock )
{
	lock->Locked = FALSE; 
}
