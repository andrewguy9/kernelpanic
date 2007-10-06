#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include"utils.h"
#include"../utils/linkedlist.h"

struct SEMAPHORE
{
	COUNT Count;
	struct LINKED_LIST WaitingThreads;
};

#endif
