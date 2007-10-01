#ifndef SCHEDULER_H
#define SCHEDULER_H

#include"../utils/link.h"

typedef void (*THREAD_MAIN) ();

enum THREAD_STATE { THREAD_STATE_RUNNING, THREAD_STATE_BLOCKED };

struct THREAD 
{
	union LINK Link;
	unsigned char Priority;
	enum THREAD_STATE State;
	void * Stack;
};

#endif
