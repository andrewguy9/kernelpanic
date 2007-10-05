#include"asrlock.h"
#include"hal.h"

BOOL ASRLock( ASR_LOCK * lock )
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

void ASRUnlock( ASR_LOCK * lock )
{
	HalDisableInterrupts();
	lock->Locked = FALSE;
	HalEnableInterrupts();
}

BOOL ASRIsLocked( ASR_LOCK * lock )
{
	BOOL value;
	HalDisableInterrupts();
	value = lock->Locked;
	HalEnableInterrupts();
	return value;
}

void ASRLockInit( ASR_LOCK * lock )
{
	lock->Locked = FALSE; 
}
