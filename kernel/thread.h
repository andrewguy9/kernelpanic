#ifndef THREAD_H
#define THREAD_H

#include"../utils/utils.h"
#include"locking.h"

enum THREAD_STATE { THREAD_STATE_RUNNING, THREAD_STATE_BLOCKED };

struct THREAD 
{
	union LINK Link;
	unsigned char Priority;
	enum THREAD_STATE State;
	struct LOCKING_CONTEXT LockingContext;
	char * Stack;
	char * StackHigh;
	char * StackLow;
	char Flag;
};


#endif
