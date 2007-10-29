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

void ResourceLock( struct RESOURCE * lock, enum RESOURCE_STATE state );

void ResourceUnlock( struct RESOURCE * lock, enum RESOURCE_STATE state );

#endif
