#ifndef RESOURCE_H
#define RESOURCE_H

#include"../utils/linkedlist.h"

enum RESOURCE_STATE { RESOURCE_SHARED, 
	RESOURCE_ESCALATING, 
	RESOURCE_EXCLUSIVE };

struct RESOURCE
{
	enum RESOURCE_STATE State;
	struct LINKED_LIST ExclusiveQueue;
	struct LINKED_LIST SharedQueue;
	COUNT NumShared;	
};

void ResourceInit( struct RESOURCE * lock );

void ResourceLockShared( struct RESOURCE * lock );
BOOL ResourceLockSharedNonBlocking( struct RESOURCE * lock );
void ResourceLockExclusive( struct RESOURCE * lock );
BOOL ResourceLockExclusiveNonBlocking( struct RESOURCE * lock );

void ResourceUnlockShared( struct RESOURCE * lock );
void ResourceUnlockExclusive( struct RESOURCE * lock );

#endif
