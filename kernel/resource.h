#ifndef RESOURCE_H
#define RESOURCE_H

#include"../utils/linkedlist.h"

enum RESOURCE_STATE 
{ 
	RESOURCE_SHARED, 
	RESOURCE_EXCLUSIVE 
};

struct RESOURCE
{
	enum RESOURCE_STATE State;
	struct LINKED_LIST WaitingThreads;
	COUNT NumShared;	
};

void ResourceInit( struct RESOURCE * lock );

void ResourceLockShared( struct RESOURCE * lock );

void ResourceLockExclusive( struct RESOURCE * lock );

void ResourceUnlockShared( struct RESOURCE * lock );

void ResourceUnlockExclusive( struct RESOURCE * lock );

#endif
