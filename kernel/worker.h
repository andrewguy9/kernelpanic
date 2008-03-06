#ifndef WORKER_H
#define WORKER_H

#include"thread.h"
#include"handler.h"

struct WORKER_ITEM
{
	struct HANDLER_OBJECT Handler;
	BOOL Queued;
	void * Context;
};

void WorkerStartup();

void WorkerCreateWorker(
		struct THREAD * thread,
		char * stack,
		unsigned int stackSize,
		char flag);

void WorkerAddItem( HANDLER_FUNCTION foo, void * context, struct WORKER_ITEM * item  );

#endif
