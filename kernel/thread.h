#ifndef THREAD_H
#define THREAD_H

#include"../utils/utils.h"
#include"blockingcontext.h"

enum THREAD_STATE { THREAD_STATE_RUNNING, THREAD_STATE_BLOCKED };

struct THREAD 
{
	union LINK Link;
	unsigned char Priority;
	enum THREAD_STATE State;
	union BLOCKING_CONTEXT BlockingContext;
	char * Stack;
#ifdef DEBUG
	char * StackHigh;
	char * StackLow;
#endif
};


#endif
