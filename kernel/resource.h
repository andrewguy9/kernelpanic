#ifndef RESOURCE_H
#define RESOURCE_H

#include"../utils/linkedlist.h"
#include"blockingcontext.h"

struct RESOURCE
{
	enum RESOURCE_STATE State;
	struct LINKED_LIST WaitingThreads;
	COUNT NumShared;	
};

void ResourceInit( struct RESOURCE * lock );

void ResourceLockShared( struct RESOURCE * lock, struct LOCKING_CONTEXT * context );

void ResourceLockExclusive( struct RESOURCE * lock, struct LOCKING_CONTEXT * context );

void ResourceUnlockShared( struct RESOURCE * lock );

void ResourceUnlockExclusive( struct RESOURCE * lock );

#endif
