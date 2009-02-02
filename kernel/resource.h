#ifndef RESOURCE_H
#define RESOURCE_H

#include"../utils/linkedlist.h"
#include"locking.h"

struct RESOURCE
{
	volatile enum RESOURCE_STATE State;
	struct LINKED_LIST WaitingThreads;
	volatile COUNT NumShared;	
};

void ResourceInit( struct RESOURCE * lock );

void ResourceLockShared( struct RESOURCE * lock, struct LOCKING_CONTEXT * context );

void ResourceLockExclusive( struct RESOURCE * lock, struct LOCKING_CONTEXT * context );

void ResourceUnlockShared( struct RESOURCE * lock );

void ResourceUnlockExclusive( struct RESOURCE * lock );

#endif
