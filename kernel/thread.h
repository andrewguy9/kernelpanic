#ifndef THREAD_H
#define THREAD_H

#include"../utils/utils.h"
#include"locking.h"
#include"context.h"
#include"stack.h"

enum THREAD_STATE { THREAD_STATE_RUNNING, THREAD_STATE_BLOCKED };

typedef void THREAD_MAIN(void * arg );

struct THREAD 
{
	union LINK Link;
	unsigned char Priority;
	enum THREAD_STATE State;
	struct LOCKING_CONTEXT LockingContext;
	struct STACK Stack;
	THREAD_MAIN * Main;
	void * Argument;
	INDEX Flag;
};


#endif
