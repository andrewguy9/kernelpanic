#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include"../utils/utils.h"
#include"../utils/linkedlist.h"

struct SEMAPHORE
{
	COUNT MaxCount;
	COUNT Count;
	struct LINKED_LIST WaitingThreads;
};

void SemaphoreInit( struct SEMAPHORE * lock, COUNT count );
void SemaphoreLock( struct SEMAPHORE * lock, COUNT count );
void SemaphoreUnlock( struct SEMAPHORE * lock, COUNT count );
#endif
