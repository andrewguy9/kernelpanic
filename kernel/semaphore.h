#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include"../utils/utils.h"
#include"../utils/linkedlist.h"

struct SEMAPHORE
{
	int Count;
	struct LINKED_LIST WaitingThreads;
};

void SemaphoreInit( struct SEMAPHORE * lock, int count );
void SemaphoreP( struct SEMAPHORE * lock );
void SemaphoreV( struct SEMAPHORE * lock );

#endif
