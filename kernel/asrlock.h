#ifndef ASRLOCK_H
#define ASRLOCK_H

#include"../utils/utils.h"

struct ASR_LOCK
{
	BOOL Locked;
};

BOOL ASRLock( ASR_LOCK * lock );
void ASRUnlock( ASR_LOCK * lock );
BOOL ASRIsLocked( ASR_LOCK * lock );
void ASRLockInit( ASR_LOCK * lock );

#endif
