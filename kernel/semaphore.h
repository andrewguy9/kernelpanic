#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include"../utils/utils.h"
#include"../utils/linkedlist.h"
#include"locking.h"

struct SEMAPHORE
{
	COUNT Count;
	struct LINKED_LIST WaitingThreads;
};

void SemaphoreInit( struct SEMAPHORE * lock, COUNT count );
void SemaphoreDown( struct SEMAPHORE * lock, struct LOCKING_CONTEXT * context );
void SemaphoreUp( struct SEMAPHORE * lock );

#endif
